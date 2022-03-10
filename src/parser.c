#include "citron.h"
#include "symbol.h"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if withInlineAsm
#    include "native-asm.h"
#endif

char* ctr_cparse_current_program;
int do_compare_locals = 0;
int all_plains_private = 0;
extern int ctr_cwlk_replace_refs;
extern char* ctr_code;
static int ctr_transform_template_expr; /* flag: indicates whether the parser is
                                           supposed to parse a templated expr */
static int uses_paramlist_item = 0;     /* flag: indicates whether a lambda is using
                                           any parameters from its param list */
int speculative_parse = 0;              /* flag: should we try to fix errors */

ctr_tnode* ctr_cparse_assignment();
ctr_tnode* ctr_cparse_block();
ctr_tnode* ctr_cparse_block_();
ctr_tnode* ctr_cparse_block_capture();
ctr_tnode* ctr_cparse_create_node();
ctr_tnode* ctr_cparse_create_node();
ctr_tnode* ctr_cparse_expr(int);
ctr_tnode* ctr_cparse_false();
ctr_tnode* ctr_cparse_fin();
ctr_tnode* ctr_cparse_list_comp();
ctr_tnode* ctr_cparse_lit_esc(int opt);
ctr_tnode* ctr_cparse_message(int mode);
ctr_tlistitem* ctr_cparse_messages(ctr_tnode*, int);
ctr_tnode* ctr_cparse_nil();
ctr_tnode* ctr_cparse_number();
ctr_tnode* ctr_cparse_parse();
ctr_tnode* ctr_cparse_popen();
ctr_tnode* ctr_cparse_pure();
ctr_tnode* ctr_cparse_comptime();
ctr_tnode* ctr_cparse_receiver();
ctr_tnode* ctr_cparse_program();
ctr_tnode* ctr_cparse_ref();
ctr_tnode* ctr_cparse_ret();
ctr_tlistitem* ctr_cparse_statement();
ctr_tnode* ctr_cparse_string();
ctr_tnode* ctr_cparse_fancy_string();
ctr_tnode* ctr_cparse_symbol();
ctr_tnode* ctr_cparse_true();
ctr_tnode* ctr_cparse_tuple(int);
ctr_tnode* ctr_fake_parse_hole();
ctr_tnode* ctr_cparse_create_generator_node_step(ctr_tnode*, ctr_tnode*,
    ctr_tnode*);
ctr_tnode* ctr_cparse_create_generator_node_simple(ctr_tnode*, ctr_tnode*);
ctr_tnode* ctr_deep_copy_ast(ctr_tnode*);

ctr_tnode* ctr_fake_parse_hole()
{
    ctr_tnode* r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_REFERENCE;
    r->vlen = 1;
    r->value = ctr_heap_allocate(1);
    strcpy(r->value, "?");
    return r;
}

int ctr_scan_inner_refs_for_(ctr_tnode* node, char* name, size_t len)
{
    switch (node->type) {
    case CTR_AST_NODE_REFERENCE: {
        int vararg = node->value[0] == '*' || node->value[0] == '&';
        if (unlikely(node->vlen == len + vararg)) {
            if (strncmp(node->value + vararg, name, len - vararg) == 0) {
                uses_paramlist_item = 1;
                return 1;
            }
        }

        return 0;
    }
    case CTR_AST_NODE_CODEBLOCK:
        return ctr_scan_inner_refs_for_(node->nodes->next->node, name, len);
    case CTR_AST_NODE_EXPRMESSAGE:
    case CTR_AST_NODE_EXPRASSIGNMENT:
        return ctr_scan_inner_refs_for_(node->nodes->node, name, len) || ctr_scan_inner_refs_for_(node->nodes->next->node, name, len);
    case CTR_AST_NODE_BINMESSAGE:
    case CTR_AST_NODE_RAW:
    case CTR_AST_NODE_NESTED:
        return ctr_scan_inner_refs_for_(node->nodes->node, name, len);
    case CTR_AST_NODE_IMMUTABLE:
    case CTR_AST_NODE_PROGRAM:
        node = node->nodes->node;
        /* Fallthrough */
    case CTR_AST_NODE_KWMESSAGE:
    case CTR_AST_NODE_INSTRLIST:
        for (ctr_tlistitem* instr = node->nodes; instr; instr = instr->next)
            if (ctr_scan_inner_refs_for_(instr->node, name, len)) {
                return 1;
            }
        // TODO X: handle listcomp
    default:
        printf("Unhandled ref scan type %d\n", node->type);
        break;
    }
    return 0;
}
int ctr_paramlist_has_name(char* namenode, size_t len)
{
    ctr_tnode* ctr_cparse_calltime_name;
    // printf("%d -- %.*s\n", ctr_cparse_calltime_name_id, len, namenode);
    if (ctr_cparse_calltime_name_id < 0 || len == 0)
        return 0;
    else {
        for (int i = ctr_cparse_calltime_name_id; i >= 0; i--) {
            ctr_cparse_calltime_name = ctr_cparse_calltime_names[i];
            // printf("checking call site %d:\n", ctr_cparse_calltime_name_id);
            // ctr_internal_debug_tree(ctr_cparse_calltime_name, 1);
            ctr_tlistitem* name = ctr_cparse_calltime_name->nodes;
            while (name) {
                // printf("  -- %d %.*s in %.*s\n", i, len, namenode, name->node->vlen,
                // name->node->value);
                if (ctr_scan_inner_refs_for_(name->node, namenode, len)) {
                    // printf("  << %d %.*s in %.*s\n", i, len, namenode,
                    // name->node->vlen, name->node->value);
                    return i;
                }
                name = name->next;
            }
        }
        return -1;
    }
}

/**
 * CTRParserEmitErrorUnexpected
 *
 * Emits a parser error and adds the file and
 * position where the error has occurred.
 * Optionally you can pass a hint to this method to
 * add some details to the error message.
 */
void ctr_cparse_emit_error_unexpected(int t, char* hint)
{
    char buf[1024];
    char* message = ctr_clex_tok_describe(t);
    sprintf(buf, "Parse %s, unexpected %s ( %s: %d )\n",
        speculative_parse ? "warning" : "error", message,
        ctr_cparse_current_program, ctr_clex_line_number + 1);
    if (ctr_cparse_quiet)
        return;
        // memcpy(ctr_last_parser_error, buf, strlen(buf));
#ifdef EXIT_ON_ERROR
    printf("%s", buf);
    if (hint) {
        printf("%s", hint);
    }
    exit(1);
#else
    if (!speculative_parse) // TODO: get an option
        CtrStdFlow = ctr_format_str("EParser %s, unexpected %s ( %s: %d)\n%s%s",
            speculative_parse ? "warning" : "error", message,
            ctr_cparse_current_program, ctr_clex_line_number + 1,
            hint ? "-> " : "", hint ? hint : "");
    else
        printf("Parser %s, unexpected %s ( %s: %d)\n%s%s",
            speculative_parse ? "warning" : "error", message,
            ctr_cparse_current_program, ctr_clex_line_number + 1,
            hint ? "-> " : "", hint ? hint : "");
#endif
}

/**
 * CTRParserCreateNode
 *
 * Creates a parser node and adds it to the source map.
 */
ctr_tnode* ctr_cparse_create_node(int type)
{
    ctr_tnode* node = (ctr_tnode*)ctr_heap_allocate_tracked(sizeof(ctr_tnode));
    if (ctr_source_mapping) {
        ctr_source_map* m = (ctr_source_map*)ctr_heap_allocate_tracked(sizeof(ctr_source_map));
        m->line = ctr_clex_line_number;
        m->node = node;
        m->p_ptr = ctr_code;
        m->e_ptr = ctr_clex_code_end;
        m->s_ptr = ctr_clex_code_init;
        if (ctr_source_map_head) {
            m->next = ctr_source_map_head;
            ctr_source_map_head = m;
        } else {
            ctr_source_map_head = m;
        }
    }
    return node;
}

static int ctr_is_binary_alternative(char const* s, long length)
{
    // return 0;
    if (length == 1)
        return 0;
    if (length == 0)
        return 0;
    for (long i = 0; i < length; i++) {
        char c = s[i];
        if (!(c == '-' || c == '+' || c == '*' || c == '&' || c == '%' || c == '$' || c == '@' || c == '!' || c == '=' || c == '"' || c == ';' || c == '/' || c == '\\' || c == '<' || c == '>' || c == '?' || c == '~' || c == '|')) {
            return 0;
        }
    }
    return 1;
}

/**
 * CTRParserMessage
 *
 * Creates the AST nodes for sending a message.
 *
 * - precedence mode 0: no argument (allows processing of unary message, binary
 * message and keyword message)
 * - precedence mode 1: as argument of keyword message (allows processing of
 * unary message and binary message)
 * - precedence mode 2: as argument of binary message (only allows processing of
 * unary message)
 */
const static char* DOLLAR_SIGN = "$";
ctr_tnode* ctr_cparse_message(int mode)
{
    long msgpartlen; /* length of part of message string */
    ctr_tnode* m;
    int t;
    char* s;
    char* msg;
    // char *lookAheadVal;
    ctr_tlistitem* li;
    ctr_tlistitem* curlistitem;
    int lookAhead;
    // int lookAheadLen = 0;
    int isBin;
    int first;

    char const* msgReplacement = "applyAll:";
    int msgReplacementLen = 9;
    t = ctr_clex_tok();
    msgpartlen = ctr_clex_tok_value_length();
    if ((msgpartlen) > 255) {
        ctr_cparse_emit_error_unexpected(t, "Message too long\n");
    }
    m = ctr_cparse_create_node(CTR_AST_NODE);
    m->type = -1;
    s = ctr_clex_tok_value();
    msg = ctr_heap_allocate_tracked(255 * sizeof(char));
    memcpy(msg, s, msgpartlen);
    lookAhead = ctr_clex_tok();
    // lookAheadVal = ctr_clex_tok_value();
    // lookAheadLen = ctr_clex_tok_value_length();
    ctr_clex_putback();
    isBin = lookAhead != CTR_TOKEN_COLON && (ctr_utf8_is_one_cluster(msg, msgpartlen) || ctr_is_binary_alternative(msg, msgpartlen));
    if (mode == 2 && isBin) {
        ctr_clex_putback();
        return m;
    }
    if (isBin) {
        m->type = CTR_AST_NODE_BINMESSAGE;
        m->value = msg;
        m->vlen = msgpartlen;
        li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
        li->node = ctr_cparse_expr(2);
        m->nodes = li;
        return m;
    }
    int replacement = 0;
    int is_bmap = 0;
    if (lookAhead == CTR_TOKEN_COLON) {
        if (mode > 0) {
            ctr_clex_putback();
            return m;
        }
        *(msg + msgpartlen) = ':';
        msgpartlen += 1;
        if ((msgpartlen) > 255) {
            ctr_cparse_emit_error_unexpected(t, "Message too long\n");
        }
        m->type = CTR_AST_NODE_KWMESSAGE;
        t = ctr_clex_tok();
        first = 1;
        while (1) {
            li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
            li->node = ctr_cparse_expr(1);
            if (first) {
                m->nodes = li;
                curlistitem = m->nodes;
                first = 0;
            } else {
                curlistitem->next = li;
                curlistitem = li;
            }
            t = ctr_clex_tok();
            if (t == CTR_TOKEN_DOT)
                break;
            if (t == CTR_TOKEN_FIN)
                break;
            if (t == CTR_TOKEN_CHAIN)
                break;
            if (t == CTR_TOKEN_PARCLOSE)
                break;
            if (t == CTR_TOKEN_TUPCLOSE)
                break;
            if (t == CTR_TOKEN_BLOCKCLOSE)
                break;
            // if (t == CTR_TOKEN_FANCY_QUOT_CLOS)
            //   break;
            if (t == CTR_TOKEN_REF) {
                long l = ctr_clex_tok_value_length();
                if ((msgpartlen + l) > 255) {
                    ctr_cparse_emit_error_unexpected(t, "Message too long\n");
                }
                memcpy((msg + msgpartlen), ctr_clex_tok_value(), l);
                msgpartlen = msgpartlen + l;
                *(msg + msgpartlen) = ':';
                msgpartlen++;
                t = ctr_clex_tok();
                if (t != CTR_TOKEN_COLON) {
                    ctr_cparse_emit_error_unexpected(t, "Expected a colon.\n");
                    ctr_clex_putback();
                }
            }
        }
        ctr_clex_putback(); /* not a colon so put back */
        m->value = msg;
        m->vlen = msgpartlen;
    } else if (t == CTR_TOKEN_BLOCKOPEN) {
        replacement = CTR_TOKEN_BLOCKCLOSE;
        goto callShorthand;
    } else if (t == CTR_TOKEN_BLOCKOPEN_MAP) {
        replacement = CTR_TOKEN_BLOCKCLOSE;
        is_bmap = 1;
        goto callShorthand;
    } else if (t == CTR_TOKEN_FANCY_QUOT_OPEN) {
        replacement = CTR_TOKEN_FANCY_QUOT_CLOS;
        msgReplacement = "process:";
        msgReplacementLen = 8;
        goto callShorthand;
    } else if (t == CTR_TOKEN_QUOTE) {
        replacement = CTR_TOKEN_QUOTE;
        msgReplacement = "process:";
        msgReplacementLen = 8;
        goto callShorthand;
    } else if (t == callShorthand->value) {
    callShorthand:;
        ctr_clex_putback();
        memcpy(msg, msgReplacement, msgReplacementLen);
        msgpartlen = msgReplacementLen;
        li = ctr_heap_allocate_tracked(sizeof(*li));
        if (nextCallLazy->value == 1 || replacement == CTR_TOKEN_BLOCKCLOSE) {
            if (!replacement)
                nextCallLazy->value--;
            if (!is_bmap) {
                if (ctr_clex_inject_token(CTR_TOKEN_INV, DOLLAR_SIGN, -2, 1)) {
                    ctr_cparse_emit_error_unexpected(
                        t, "lazy call cannot be instantiated at this state");
                    return NULL;
                }
                li->node = ctr_cparse_lit_esc(replacement ?: callShorthand->value_e);
            } else {
                int texpr = ctr_transform_template_expr;
                ctr_transform_template_expr = 1;
                ctr_tnode* block = ctr_cparse_block();
                ctr_transform_template_expr = texpr;
                ctr_tnode* v = ctr_cparse_create_node(CTR_AST_NODE);
                v->type = CTR_AST_NODE_RAW;
                v->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
                v->nodes->node = ctr_deep_copy_ast(block);
                block = v;
                block->modifier = 1;
                ctr_tnode* arg = ctr_cparse_create_node(CTR_AST_NODE);
                ctr_tlistitem* tl0 = ctr_heap_allocate(sizeof *tl0);
                arg->type = CTR_AST_NODE_IMMUTABLE;
                arg->nodes = tl0;
                tl0->next = NULL;
                tl0->node = ctr_cparse_create_node(CTR_AST_NODE);
                tl0->node->type = CTR_AST_NODE_NESTED;
                ctr_tlistitem* tl1 = ctr_heap_allocate(sizeof *tl1);
                tl0->node->nodes = tl1;
                tl1->next = NULL;
                tl1->node = block;
                li->node = arg;
            }
        } else if (nextCallLazy->value > 1) {
            nextCallLazy->value--;
            int texpr_res = ctr_transform_template_expr;
            ctr_transform_template_expr = 0;
            li->node = ctr_cparse_tuple(replacement ?: callShorthand->value_e);
            ctr_transform_template_expr = texpr_res;
        } else {
            int texpr_res = ctr_transform_template_expr;
            ctr_transform_template_expr = 0;
            if (replacement == CTR_TOKEN_FANCY_QUOT_CLOS)
                li->node = ctr_cparse_fancy_string();
            if (replacement == CTR_TOKEN_QUOTE)
                li->node = ctr_cparse_string();
            else
                li->node = ctr_cparse_tuple(replacement ?: callShorthand->value_e);
            ctr_transform_template_expr = texpr_res;
        }
        replacement = 0;
        m->type = CTR_AST_NODE_KWMESSAGE;
        m->nodes = li;
        m->value = msg;
        m->vlen = msgpartlen;
        m->modifier = -2; // chain nothing
    } else {
        m->type = CTR_AST_NODE_UNAMESSAGE;
        m->value = msg;
        m->vlen = msgpartlen;
    }
    return m;
}

/**
 * CTRParserMessages
 *
 * Manages the creation of nodes to send a message, uses CTRParserMessage
 * to create the actual nodes.
 */
ctr_tlistitem* ctr_cparse_messages(ctr_tnode* r, int mode)
{
    int t = ctr_clex_tok();
    ctr_tlistitem* pli = NULL;
    ctr_tlistitem* li = NULL;
    ctr_tlistitem* fli = NULL;
    int first = 1;
    ctr_tnode* node = NULL;
    /* explicit chaining (,) only allowed for keyword message: Console write: 3
     * factorial, write: 3 factorial is not possible otherwise. */
    while (t == CTR_TOKEN_REF || (t == CTR_TOKEN_CHAIN && node && node->type == CTR_AST_NODE_KWMESSAGE && node->modifier != -2) || (t == callShorthand->value) || (t == CTR_TOKEN_BLOCKOPEN || t == CTR_TOKEN_BLOCKOPEN_MAP) || (t == CTR_TOKEN_FANCY_QUOT_OPEN) || (t == CTR_TOKEN_QUOTE)) {
        if (t == CTR_TOKEN_CHAIN) {
            t = ctr_clex_tok();
            if (t != CTR_TOKEN_REF) {
                ctr_cparse_emit_error_unexpected(t, "Expected message.\n");
                if (speculative_parse)
                    if (ctr_clex_inject_token(CTR_TOKEN_REF, ctr_clex_tok_value() ?: "?",
                            ctr_clex_tok_value_length() ?: 11, 11)) {
                        ctr_cparse_emit_error_unexpected(
                            t, "Speculative parsing failed, not enough vector space\n");
                        return NULL;
                    }
            }
        }
        li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
        ctr_clex_putback();
        node = ctr_cparse_message(mode);
        if (node->type == -1) {
            if (first) {
                return NULL;
            }
            ctr_clex_tok();
            break;
        }
        li->node = node;
        if (first) {
            first = 0;
            pli = li;
            fli = li;
        } else {
            pli->next = li;
            pli = li;
        }
        t = ctr_clex_tok();
    }
    ctr_clex_putback();
    return fli;
}

/**
 * CTRParserListComp
 *
 * generates a node to represent a list comprehension
 *
 * @param ctr_tnode * main_expr -- the main expression of the comprehension
 * Lexer state at this point:
 *  [ expression ,, expression* ,, expression* ]
 *                 ^
 */
ctr_tnode* ctr_cparse_list_comp(ctr_tnode* main_expr)
{
    ctr_tnode* r;
    ctr_tlistitem *part0, *part1, *part2;

    int t;

    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LISTCOMP;

    part0 = ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    part1 = ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    part2 = ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));

    /*
       r
       |_ part0 -- main expression
       |_ part1 -- generator
       |          |_ gen0 -- (kwmessage?) name || expr
       |          | (kwmessage?) |_ expr
       |          |
       |          |_ gen1, etc
       |
       |_ part2 -- predicate
       |_ p0
       |_ p1, etc
     */
    r->nodes = part0;
    part0->next = part1;
    part1->next = part2;

    part0->node = main_expr; // save main expression
    t = ctr_clex_tok();
    ctr_clex_putback();
    //[ expression ,,, predicate* ] (skipped generators)
    //               ^
    if (t == CTR_TOKEN_CHAIN) {
        ctr_clex_tok(); // eat the ','
        part1->node = NULL;
        goto parse_predicates;
    }
    //[ expression ,, (name: expression)* (,,)? expression* ]
    //                ^
    // parse a series of expressions, separated by CHAIN, put into part1
    ctr_tnode* gen = ctr_heap_allocate_tracked(sizeof(*gen));
    gen->nodes = ctr_heap_allocate_tracked(sizeof(*part1));
    part1->node = gen;
    part1 = gen->nodes;
    part1->node = ctr_cparse_expr(0);
    while ((t = ctr_clex_tok()) == CTR_TOKEN_CHAIN) {
        if (ctr_clex_tok() == CTR_TOKEN_CHAIN) {
            ctr_clex_tok();
            break; // we're going into the predicates now
        }
        ctr_clex_putback();
        part1->next = ctr_heap_allocate_tracked(sizeof(*part1));
        part1 = part1->next;
        part1->node = ctr_cparse_expr(0);
    }
    ctr_clex_putback();
parse_predicates:;
    t = ctr_clex_tok();
    ctr_clex_putback();
    // [ expression ,, expression* ,, ]
    //                               ^
    if (t == CTR_TOKEN_TUPCLOSE) // no predicates, so a length-one tuple if it has
                                 // no gens
    {
        ctr_clex_tok(); // eat the ']'
        // printf("%s\n", ctr_code);
        if (r->nodes->next->node == NULL) { // didn't have generators
            // replace with a tuple and move on
            r->type = CTR_AST_NODE_IMMUTABLE;
            r->nodes->next = NULL;
        } else {
            r->nodes->next->next->node = NULL;
        }
        return r;
    }
    // [ expression ,, expression* ,, expression+ ]
    //                                ^
    // parse a series of expressions, separated by CHAIN, put into part2
    ctr_tnode* pred = ctr_heap_allocate_tracked(sizeof(*pred));
    pred->nodes = ctr_heap_allocate_tracked(sizeof(*part2));
    part2->node = pred;
    part2 = pred->nodes;
    part2->node = ctr_cparse_expr(0);
    while ((t = ctr_clex_tok()) == CTR_TOKEN_CHAIN) {
        part2->next = ctr_heap_allocate_tracked(sizeof(*part1));
        part2 = part2->next;
        part2->node = ctr_cparse_expr(0);
    }
    if (t != CTR_TOKEN_TUPCLOSE) {
        // bitch about it
        ctr_cparse_emit_error_unexpected(t, "Expected a ']'\n");
    }
    return r;
}

/**
 * CTRParserLiteralEscape
 *
 * Generates a node for literal escapes
 */
ctr_tnode* ctr_cparse_lit_esc(int opt)
{
    ctr_tnode *r, *v;
    ctr_clex_tok();
    char texpr_res = ctr_transform_template_expr;
    int len = ctr_clex_tok_value_length();
    int unescape = 0, quote = 0;
    switch (len) {
    case -1:                             //$()
        ctr_transform_template_expr = 1; // flip it
        r = ctr_cparse_popen();
        v = r->nodes->node;
        ctr_transform_template_expr = texpr_res;
        ctr_heap_free(
            r); // this node is not needed, we remove the parens in the expression
        r = v;
        break;
    case -2: // $[]
        ctr_transform_template_expr = 2;
        r = ctr_cparse_tuple(opt);
        ctr_transform_template_expr = texpr_res;
        break;
    case -4: // $'()
        quote = 1;
        /* Fallthrough */
    case -3: // $!
    {
        int t = ctr_clex_tok();
        ctr_clex_putback();
        ctr_transform_template_expr = 0;
        if (t == CTR_TOKEN_PAROPEN) {
            r = ctr_cparse_popen();
            ctr_tnode* f = r->nodes->node;
            ctr_heap_free(r);
            r = f;
        } else
            r = ctr_cparse_expr(0);
        unescape = 1;
        ctr_transform_template_expr = texpr_res;
        break;
    }
    case -5: {
        int t = ctr_clex_tok();
        ctr_clex_putback();
        ctr_transform_template_expr = 0;
        if (t == CTR_TOKEN_PAROPEN) {
            r = ctr_cparse_popen();
            r->type = CTR_AST_NODE_EMBED;
            r->modifier = 0;
        } else {
            r = ctr_cparse_create_node(CTR_AST_NODE);
            r->type = CTR_AST_NODE_EMBED;
            r->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
            r->nodes->node = ctr_cparse_expr(0); // temporarily
            r->modifier = 0;
        }
        unescape = 1;
        ctr_transform_template_expr = texpr_res;
        break;
    }
    default:
        ctr_cparse_emit_error_unexpected(
            ctr_clex_tok(), "Expected any of '(', '[', '!', '`', '\''\n");
        return NULL;
    }
    v = ctr_cparse_create_node(CTR_AST_NODE);
    v->type = CTR_AST_NODE_RAW;
    v->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
    v->nodes->node = ctr_deep_copy_ast(r);
    if (!unescape)
        v->modifier = -len;
    else {
        v->modifier = -quote; // 0 or -1
    }
    return v;
}

/**
 * CTRParserTuple
 *
 * Generates a node to represent an immutable array (tuple)
 */
ctr_tnode* ctr_cparse_tuple(int ending_tok)
{
    ctr_tnode* r;
    ctr_tlistitem* codeBlockPart1;
    ctr_tnode* paramList;
    ctr_tlistitem* previousListItem;
    int t;
    t = ctr_clex_tok(); // eat the [
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_IMMUTABLE;
    codeBlockPart1 = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    r->nodes = codeBlockPart1;
    paramList = ctr_cparse_create_node(CTR_AST_NODE);
    codeBlockPart1->node = paramList;
    paramList->type = CTR_AST_NODE_NESTED;
    t = ctr_clex_tok();
    ctr_clex_putback();
    if (t == ending_tok) {
        ctr_clex_tok(); // eat the ending ]
        return r;
    }
    ctr_tlistitem* paramListItem = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    paramList->nodes = paramListItem;
    paramListItem->node = ctr_cparse_expr(0);
    if (ctr_transform_template_expr == 2) {
        ctr_tnode* nd = ctr_cparse_create_node(CTR_AST_NODE);
        nd->type = CTR_AST_NODE_RAW;
        nd->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
        nd->nodes->node = paramListItem->node;
        nd->modifier = 1;
        paramListItem->node = nd;
    }
    previousListItem = paramListItem;

    int restore_id = ctr_clex_save_state(); // save lexer state
    if (restore_id == -1) {
        ctr_cparse_emit_error_unexpected(0, "Lexer stack overflow");
        return NULL;
    }
    // try for a list comprehension: expect ,,
    t = ctr_clex_tok();
    if (t != CTR_TOKEN_CHAIN && t != ending_tok) { // common element
        ctr_cparse_emit_error_unexpected(t, "Expected a ',' or a ']'\n");
        return r;
    }
    if (t == ending_tok) {
        return r;
    }
    t = ctr_clex_tok();
    if (t == CTR_TOKEN_CHAIN) { // list comp
        return ctr_cparse_list_comp(r->nodes->node);
    }
    ctr_clex_restore_state(restore_id); // restore after checking for listcomp

    while ((t = ctr_clex_tok()) == CTR_TOKEN_CHAIN) {
        /* okay we have new expr, parse it */
        ctr_tlistitem* paramListItem = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
        paramListItem->node = ctr_cparse_expr(0);
        if (ctr_transform_template_expr == 2) {
            ctr_tnode* nd = ctr_cparse_create_node(CTR_AST_NODE);
            nd->type = CTR_AST_NODE_RAW;
            nd->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
            nd->nodes->node = paramListItem->node;
            nd->modifier = 1;
            paramListItem->node = nd;
        }
        previousListItem->next = paramListItem;
        previousListItem = paramListItem;
    }
    if (t != ending_tok)
        ctr_cparse_emit_error_unexpected(t, "Expected a ']'.");
    return r;
}

/**
 * CTRParserSymbol
 *
 * Generates a symbol or pulls it from the static symbol list.
 */
ctr_tnode* ctr_cparse_symbol()
{
    ctr_tnode* r;
    ctr_tlistitem* li;
    r = ctr_cparse_create_node(CTR_AST_NODE);
    int t = ctr_clex_tok();
    if (t != CTR_TOKEN_SYMBOL)
        return NULL;
    ctr_clex_tok();
    r->type = CTR_AST_NODE_SYMBOL;
    li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    ctr_size vlen = ctr_clex_tok_value_length();
    r->value = (char*)ctr_get_or_create_symbol_table_entry(ctr_clex_tok_value(), vlen);
    r->vlen = -1; // sig
    return r;
}

/**
 * CTRParserPOpen
 *
 * Generates a set of nested nodes.
 */
ctr_tnode* ctr_cparse_popen()
{
    ctr_tnode* r;
    ctr_tlistitem* li;
    int t;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_NESTED;
    li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    r->nodes = li;
    li->node = ctr_cparse_expr(0);
    t = ctr_clex_tok();
    if (t != CTR_TOKEN_PARCLOSE) {
        ctr_cparse_emit_error_unexpected(t, "Expected a ')'.\n");
    }
    return r;
}

#if withInlineAsm
ctr_tnode* ctr_cparse_intern_asm_block_()
{
    /**
     *
     * {asm att|intel? (STRING)? _asm_}
     *
     */
    asm_arg_info_t arginfo[64];

    int t = ctr_clex_tok();
    int argidx = -1;
    while (t == CTR_TOKEN_COLON) {
        // arguments to asm _must_ be numbers,
        // they must be used prepended with a colon
        // in the output/input constraints
        // no more than 4 arguments will be processed
        t = ctr_clex_tok();
        if (t != CTR_TOKEN_REF) {
            ctr_cparse_emit_error_unexpected(t, "Expected an argument name\n");
            return speculative_parse ? ctr_fake_parse_hole() : NULL;
        }
        int len = ctr_clex_tok_value_length();
        char* val = ctr_clex_tok_value();
        for (int i = 0; i < len; i++)
            if (!isalpha(val[i])) {
                ctr_cparse_emit_error_unexpected(
                    t, "asm block arguments must contain only alpha characters\n");
                return speculative_parse ? ctr_fake_parse_hole() : NULL;
            }
        argidx++;
        enum AsmArgType _ty = ASM_ARG_TY_DBL;
        if (len == 3 && strncmp(val, "int", 3) == 0)
            _ty = ASM_ARG_TY_INT;
        if (len == 3 && strncmp(val, "str", 3) == 0)
            _ty = ASM_ARG_TY_STR;
        arginfo[argidx].ty = _ty;
        t = ctr_clex_tok();
    }
    char* constraint = "\0";
    int att = 1;
    if (t == CTR_TOKEN_REF) {
        int len = ctr_clex_tok_value_length();
        char* tok = ctr_clex_tok_value();
        if (len == 5 && strncasecmp(tok, "intel", 5) == 0)
            att = 0;
        else if (!((len == 4 && strncasecmp(tok, "at&t", 4) == 0) || (len == 3 && strncasecmp(tok, "att", 3) == 0))) {
            ctr_cparse_emit_error_unexpected(
                t, "Expected literal name att|at&t|intel\n");
            return speculative_parse ? ctr_fake_parse_hole() : NULL;
        }
        t = ctr_clex_tok();
    }
    if (t == CTR_TOKEN_PAROPEN) {
        char* begin = ctr_code;
        char* end = ctr_clex_scan(')');
        if (!end) {
            ctr_cparse_emit_error_unexpected(
                t, "Expected a ')' to end the asm constraint block\n");
            return speculative_parse ? ctr_fake_parse_hole() : NULL;
        }
        constraint = ctr_heap_allocate(end - begin + 1);
        memcpy(constraint, begin, end - begin);
        constraint[end - begin] = 0;
        ctr_code++;
    }
    char* asm_begin = ctr_code;
    char* asm_end = ctr_clex_scan('}');
    ctr_clex_tok();
    if (!asm_end) {
        ctr_cparse_emit_error_unexpected(
            t, "Expected a '}' to end the native block\n");
        return speculative_parse ? ctr_fake_parse_hole() : NULL;
    }
    void* fn = ctr_cparse_intern_asm_block(
        /* start = */ asm_begin,
        /* end = */ asm_end,
        /* constraint= */ constraint,
        /* offset = */ (int)&((ctr_object*)NULL)->value.nvalue,
        /* argc = */ argidx,
        /* arginfo = */ &arginfo,
        /* dialect = */ att);
    if (constraint[0])
        ctr_heap_free(constraint);
    if (!fn) {
        ctr_cparse_emit_error_unexpected(t, "Invalid assembly\n");
        return NULL;
    }
    ctr_tnode* node = ctr_cparse_create_node(CTR_AST_NODE);
    node->type = CTR_AST_NODE_NATIVEFN;
    node->value = (char*)fn;
    node->vlen = 0;
    // src
    node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
    node->nodes->node = ctr_cparse_create_node(CTR_AST_NODE);
    node->nodes->node->type = CTR_AST_NODE_LTRSTRING;
    node->nodes->node->vlen = asm_end - asm_begin;
    node->nodes->node->value = ctr_heap_allocate(node->nodes->node->vlen + 1);
    memcpy(node->nodes->node->value, asm_begin, asm_end - asm_begin);
    node->nodes->node->value[node->nodes->node->vlen] = 0;
    // TODO: constraints, types and dialect
    return node;
}
#endif

/**
 * CTRParserBlock
 *
 * Generates a set of AST nodes to represent a block of code.
 */

ctr_tnode* ctr_cparse_block_(int autocap);
ctr_tnode* ctr_cparse_block_pv(int autocap, int error);
// __attribute__ ((always_inline))
ctr_tnode* ctr_cparse_block() { return ctr_cparse_block_(0); }

// __attribute__ ((always_inline))
ctr_tnode* ctr_cparse_block_capture() { return ctr_cparse_block_(1); }

ctr_tnode* ctr_cparse_block_(int autocap)
{
    return ctr_cparse_block_pv(autocap, 0);
}

ctr_tnode* ctr_cparse_block_pv(int autocap, int error)
{
    ctr_tnode* r;
    ctr_tlistitem* codeBlockPart1;
    ctr_tlistitem* codeBlockPart2;
    ctr_tnode* paramList;
    ctr_tnode* codeList;
    ctr_tlistitem* previousListItem;
    ctr_tlistitem* previousCodeListItem;
    int t;
    int first;
    ctr_clex_tok();
    t = ctr_clex_tok();
#if withInlineAsm
    if ((extensionsPra->value & CTR_EXT_ASM_BLOCK) == CTR_EXT_ASM_BLOCK && t == CTR_TOKEN_REF && ctr_clex_tok_value_length() == 3 && strncmp(ctr_clex_tok_value(), "asm", 3) == 0)
        return ctr_cparse_intern_asm_block_();
#endif
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_CODEBLOCK;
    codeBlockPart1 = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    r->nodes = codeBlockPart1;
    codeBlockPart2 = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    r->nodes->next = codeBlockPart2;
    paramList = ctr_cparse_create_node(CTR_AST_NODE);
    codeList = ctr_cparse_create_node(CTR_AST_NODE);
    codeBlockPart1->node = paramList;
    codeBlockPart2->node = codeList;
    paramList->type = CTR_AST_NODE_PARAMLIST;
    codeList->type = CTR_AST_NODE_INSTRLIST;
    first = 1;
    while (t == CTR_TOKEN_COLON) {
        /* okay we have a new parameter, load it */
        t = ctr_clex_tok();
        ctr_tlistitem* paramListItem = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
        ctr_tnode* paramItem;
        if (t == CTR_TOKEN_REF) {
            paramItem = ctr_cparse_create_node(CTR_AST_NODE);
            long l = ctr_clex_tok_value_length();
            paramItem->value = ctr_heap_allocate_tracked(sizeof(char) * l);
            paramItem->type = CTR_AST_NODE_REFERENCE;
            memcpy(paramItem->value, ctr_clex_tok_value(), l);
            paramItem->vlen = l;
        } else if (t == CTR_TOKEN_PAROPEN) {
            ctr_clex_putback();
            paramItem = ctr_cparse_popen();
        } else {
            paramItem = NULL;
        }
        paramListItem->node = paramItem;
        if (first) {
            paramList->nodes = paramListItem;
            previousListItem = paramListItem;
            first = 0;
        } else {
            previousListItem->next = paramListItem;
            previousListItem = paramListItem;
        }
        t = ctr_clex_tok();
    }
    first = 1;
    int oldallpl;
    int olddcl;
    int oldcalltime;
    if (!ctr_transform_template_expr) {
        oldallpl = all_plains_private;
        olddcl = do_compare_locals;
        do_compare_locals = autocap;
        oldcalltime = ctr_cparse_calltime_name_id;
        ctr_cparse_calltime_names[++ctr_cparse_calltime_name_id] = paramList;
        // printf("call site %d:\n", ctr_cparse_calltime_name_id);
        // ctr_internal_debug_tree(paramList, 1);
        all_plains_private = autocap;
    }
    if (ctr_transform_lambda_shorthand) {
        ctr_transform_lambda_shorthand = 0;
        ctr_tlistitem* codeListItem;
        ctr_tnode *codeNode, *innerNode;
        ctr_clex_putback();
        codeListItem = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
        uses_paramlist_item = 0;
        innerNode = ctr_cparse_expr(0); // parse a single expression
        if ((extensionsPra->value & CTR_EXT_PURE_FS) == CTR_EXT_PURE_FS && !uses_paramlist_item) {
            codeNode = ctr_cparse_create_node(CTR_AST_NODE);
            codeNode->type = CTR_AST_NODE_EXPRASSIGNMENT;
            codeNode->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
            ctr_tnode* frozen_ref = ctr_heap_allocate(sizeof(ctr_tnode));
            frozen_ref->value = NULL;
            frozen_ref->vlen = 0;
            frozen_ref->type = CTR_AST_NODE_REFERENCE;
            frozen_ref->modifier = 5; /* static, ignore assignment, only evaluate result */
            codeNode->nodes->node = frozen_ref;
            codeNode->nodes->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
            codeNode->nodes->next->node = innerNode;
        } else {
            codeNode = innerNode;
        }
        codeListItem->node = codeNode;
        codeList->nodes = codeListItem;
        previousCodeListItem = codeListItem;
    } else {
        while ((first || t == CTR_TOKEN_DOT)) {
            ctr_tlistitem* codeListItem;
            ctr_tnode* codeNode;
            if (first) {
                ctr_clex_putback();
            }
            t = ctr_clex_tok();
            if (t == CTR_TOKEN_BLOCKCLOSE)
                break;
            ctr_clex_putback();
            codeListItem = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
            if (t == CTR_TOKEN_RET) {
                codeNode = ctr_cparse_ret();
            } else {
                codeNode = ctr_cparse_expr(0);
            }
            codeListItem->node = codeNode;
            if (first) {
                codeList->nodes = codeListItem;
                previousCodeListItem = codeListItem;
                first = 0;
            } else {
                previousCodeListItem->next = codeListItem;
                previousCodeListItem = codeListItem;
            }
            t = ctr_clex_tok();
            if (t != CTR_TOKEN_DOT && !autocap) {
                ctr_cparse_emit_error_unexpected(t, "Expected a dot (.).\n");
            }
        }
    }
    if (!ctr_transform_template_expr) {
        all_plains_private = oldallpl;
        do_compare_locals = olddcl;
        ctr_cparse_calltime_name_id = oldcalltime;
    }
    r->modifier = /*CTR_MODIFIER_AUTOCAPTURE */ autocap == 1;
    return r;
}

/**
 * CTRParserReference
 *
 * Generates the nodes to respresent a variable or property.
 */
ctr_tnode* ctr_cparse_ref()
{
    ctr_tnode* r;
    char* tmp;
    ctr_clex_tok();
    if (ctr_clex_tok_value_length() == 4 && strncmp(ctr_clex_tok_value(), "pure", 4) == 0) {
        // pure { ... }
        int t = ctr_clex_tok();
        if (t != CTR_TOKEN_BLOCKOPEN) {
            ctr_clex_putback();
            ctr_clex_putback();
            ctr_clex_tok();
            goto the_else;
        }
        return ctr_cparse_pure();
    }
    if (ctr_clex_tok_value_length() == 9 && strncmp(ctr_clex_tok_value(), "@comptime", 9) == 0)
        return ctr_cparse_comptime();
the_else:;
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_REFERENCE;
    r->vlen = ctr_clex_tok_value_length();
    tmp = ctr_clex_tok_value();
    int calltimeidv;
    if (all_plains_private) {
        if (!do_compare_locals)
            r->modifier = 3;
        else if ((calltimeidv = ctr_paramlist_has_name(tmp, r->vlen)) == ctr_cparse_calltime_name_id) {
            r->modifier = 0;
            // printf("call site %d has %.*s\n", calltimeidv, r->vlen, tmp);
        } else {
            r->modifier = 3;
            // printf("call site %d has %.*s, not %d\n", calltimeidv, r->vlen, tmp,
            // ctr_cparse_calltime_name_id);
        }
    }
    if (strncmp(ctr_clex_keyword_my, tmp, ctr_clex_keyword_my_len) == 0 && r->vlen == ctr_clex_keyword_my_len) {
        int t = ctr_clex_tok();
        if (t != CTR_TOKEN_REF) {
            ctr_cparse_emit_error_unexpected(
                t, "'My' should always be followed by a property name!\n");
            if (speculative_parse)
                if (ctr_clex_inject_token(CTR_TOKEN_REF, ctr_clex_tok_value(),
                        ctr_clex_tok_value_length(),
                        ctr_clex_tok_value_length())) {
                    ctr_cparse_emit_error_unexpected(
                        t, "Speculative parsing failed, not enough vector space\n");
                    return NULL;
                }
        }
        tmp = ctr_clex_tok_value();
        r->modifier = 1;
        r->vlen = ctr_clex_tok_value_length();
    }
    if (strncmp(ctr_clex_keyword_var, tmp, ctr_clex_keyword_var_len) == 0 && r->vlen == ctr_clex_keyword_var_len) {
        int t = ctr_clex_tok();
        if (t != CTR_TOKEN_REF) {
            ctr_cparse_emit_error_unexpected(
                t, "'var' should always be follwed by a property name!\n");
            if (speculative_parse)
                if (ctr_clex_inject_token(CTR_TOKEN_REF, ctr_clex_tok_value(),
                        ctr_clex_tok_value_length(),
                        ctr_clex_tok_value_length())) {
                    ctr_cparse_emit_error_unexpected(
                        t, "Speculative parsing failed, not enough vector space\n");
                    return NULL;
                }
        }
        tmp = ctr_clex_tok_value();
        r->modifier = 2;
        r->vlen = ctr_clex_tok_value_length();
    }
    if (strncmp(ctr_clex_keyword_const, tmp, ctr_clex_keyword_const_len) == 0 && r->vlen == ctr_clex_keyword_const_len) {
        int t = ctr_clex_tok();
        if (t != CTR_TOKEN_REF) {
            ctr_cparse_emit_error_unexpected(
                t,
                "'const' must always be followed by a single reference/property\n");
            if (speculative_parse)
                if (ctr_clex_inject_token(CTR_TOKEN_REF, ctr_clex_tok_value(),
                        ctr_clex_tok_value_length(),
                        ctr_clex_tok_value_length())) {
                    ctr_cparse_emit_error_unexpected(
                        t, "Speculative parsing failed, not enough vector space\n");
                    return NULL;
                }
        }
        tmp = ctr_clex_tok_value();
        r->modifier = 3;
        r->vlen = ctr_clex_tok_value_length();
    }
    if (strncmp(ctr_clex_keyword_static, tmp, ctr_clex_keyword_static_len) == 0 && r->vlen == ctr_clex_keyword_static_len) {
        if ((extensionsPra->value & CTR_EXT_FROZEN_K) != CTR_EXT_FROZEN_K) {
            ctr_cparse_emit_error_unexpected(
                CTR_TOKEN_REF,
                "XFrozen extension is required to use the `" CTR_DICT_STATIC
                "' modifier");
            return NULL;
        }
        int t = ctr_clex_tok();
        if (t != CTR_TOKEN_REF) {
            ctr_cparse_emit_error_unexpected(
                t, "'" CTR_DICT_STATIC "' must always be followed by a single property name\n");
            if (speculative_parse)
                if (ctr_clex_inject_token(CTR_TOKEN_REF, ctr_clex_tok_value(),
                        ctr_clex_tok_value_length(),
                        ctr_clex_tok_value_length())) {
                    ctr_cparse_emit_error_unexpected(
                        t, "Speculative parsing failed, not enough vector space\n");
                    return NULL;
                }
        }
        tmp = ctr_clex_tok_value();
        r->modifier = 4;
        r->vlen = ctr_clex_tok_value_length();
        /* check precondition just in case */
        t = ctr_clex_tok();
        ctr_clex_putback();
        if (t != CTR_TOKEN_ASSIGNMENT) {
            ctr_cparse_emit_error_unexpected(
                t, "'" CTR_DICT_STATIC "' variable must be in an assignment");
            if (speculative_parse)
                if (ctr_clex_inject_token(CTR_TOKEN_ASSIGNMENT, "is", 2, 2)) {
                    ctr_cparse_emit_error_unexpected(
                        t, "Speculative parsing failed, not enough vector space\n");
                    return NULL;
                }
        }
    }

    r->value = ctr_heap_allocate_tracked(r->vlen);
    memcpy(r->value, tmp, r->vlen);
    return r;
}

/**
 * CTRParserString
 *
 * Generates a node to represent a string.
 */
ctr_tnode* ctr_cparse_string()
{
    ctr_tnode* r;
    char* n;
    ctr_size vlen;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRSTRING;
    n = ctr_clex_readstr();
    vlen = ctr_clex_tok_value_length();
    r->value = ctr_heap_allocate_tracked(sizeof(char) * vlen);
    memcpy(r->value, n, vlen);
    r->vlen = vlen;
    ctr_clex_tok(); /* eat trailing quote. */
    return r;
}

ctr_tnode* ctr_cparse_fancy_string()
{
    ctr_tnode* r;
    char* n;
    ctr_size vlen;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRSTRING;
    n = ctr_clex_readfstr();
    vlen = ctr_clex_tok_value_length();
    r->value = ctr_heap_allocate_tracked(sizeof(char) * vlen);
    memcpy(r->value, n, vlen);
    r->vlen = vlen;
    ctr_clex_tok(); /* eat trailing quote. */
    return r;
}
/**
 * CTRParserCreateSimpleRangeGenerator
 *
 * Generates a node to represent a simple range
 */
ctr_tnode* ctr_cparse_create_generator_node_simple(ctr_tnode* from,
    ctr_tnode* to)
{
    ctr_tnode *root = ctr_cparse_create_node(CTR_AST_NODE),
              *ref = ctr_cparse_create_node(CTR_AST_NODE),
              *msg = ctr_cparse_create_node(CTR_AST_NODE);
    ctr_tlistitem *li = ctr_heap_allocate(sizeof(*li)), *li2,
                  *li3 = ctr_heap_allocate(sizeof(*li3)),
                  *msgli0 = ctr_heap_allocate(sizeof(*li3)),
                  *msgli1 = ctr_heap_allocate(sizeof(*li3));
    msgli0->node = from;
    msgli0->next = msgli1;
    msgli1->node = to;
    msgli1->next = NULL;
    li->node = ref;
    li2 = li->next = ctr_heap_allocate(sizeof(*li));
    li2->node = msg;
    msg->nodes = li3;
    static char const *msgname = "from:to:", *refname = "Generator";
    msg->vlen = strlen(msgname);
    msg->value = ctr_heap_allocate(msg->vlen);
    memcpy(msg->value, msgname, msg->vlen);
    msg->nodes = msgli0;
    msg->type = CTR_AST_NODE_KWMESSAGE;

    ref->vlen = strlen(refname);
    ref->value = ctr_heap_allocate(ref->vlen);
    memcpy(ref->value, refname, ref->vlen);
    ref->type = CTR_AST_NODE_REFERENCE;

    root->type = CTR_AST_NODE_EXPRMESSAGE;
    root->nodes = li;
    return root;
}

/**
 * CTRParserCreateStepRangeGenerator
 *
 * Generates a node to represent a range with a step
 */
ctr_tnode* ctr_cparse_create_generator_node_step(ctr_tnode* from,
    ctr_tnode* step,
    ctr_tnode* to)
{
    ctr_tnode *root = ctr_cparse_create_node(CTR_AST_NODE),
              *ref = ctr_cparse_create_node(CTR_AST_NODE),
              *msg = ctr_cparse_create_node(CTR_AST_NODE);
    ctr_tlistitem *li = ctr_heap_allocate(sizeof(*li)), *li2,
                  *li3 = ctr_heap_allocate(sizeof(*li3)),
                  *msgli0 = ctr_heap_allocate(sizeof(*li3)),
                  *msgli1 = ctr_heap_allocate(sizeof(*li3)),
                  *msgli2 = ctr_heap_allocate(sizeof(*li3));
    msgli0->node = from;
    msgli0->next = msgli1;
    msgli1->node = to;
    msgli1->next = msgli2;
    msgli2->node = step;
    msgli2->next = NULL;
    li->node = ref;
    li2 = li->next = ctr_heap_allocate(sizeof(*li));
    li2->node = msg;
    msg->nodes = li3;
    static char const *msgname = "from:to:step:", *refname = "Generator";
    msg->vlen = strlen(msgname);
    msg->value = ctr_heap_allocate(msg->vlen);
    memcpy(msg->value, msgname, msg->vlen);
    msg->nodes = msgli0;
    msg->type = CTR_AST_NODE_KWMESSAGE;
    ref->vlen = strlen(refname);
    ref->value = ctr_heap_allocate(ref->vlen);
    memcpy(ref->value, refname, ref->vlen);
    ref->type = CTR_AST_NODE_REFERENCE;
    root->type = CTR_AST_NODE_EXPRMESSAGE;
    root->nodes = li;
    return root;
}

/**
 * CTRParserNumber
 *
 * Generates a node to represent a number, or if NUMBER DOT DOT DOT NUMBER |
 * NUMBER DOT DOT NUMBER DOT DOT NUMBER creates a generator
 */
void ctr_internal_debug_tree(ctr_tnode* ti, int indent);
ctr_tnode* ctr_cparse_number()
{
    char* n;
    ctr_tnode* r;
    long l;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRNUM;
    n = ctr_clex_tok_value();
    l = ctr_clex_tok_value_length();
    r->value = ctr_heap_allocate_tracked(sizeof(char) * l);
    memcpy(r->value, n, l);
    r->vlen = l;
    int t = ctr_clex_tok();
    if (t == CTR_TOKEN_DOT) {
        // NUMBER DOT
        t = ctr_clex_tok();
        if (t == CTR_TOKEN_DOT) {
            // NUMBER DOT DOT
            t = ctr_clex_tok();
            if (t == CTR_TOKEN_NUMBER) { // it's a range
                // NUMBER DOT DOT NUMBER
                char* ne;
                ctr_tnode* re = ctr_cparse_create_node(CTR_AST_NODE);
                long le;
                ne = ctr_clex_tok_value();
                le = ctr_clex_tok_value_length();
                re->value = ctr_heap_allocate_tracked(sizeof(char) * le);
                re->type = CTR_AST_NODE_LTRNUM;
                memcpy(re->value, ne, le);
                re->vlen = le;
                t = ctr_clex_tok();
                if (t == CTR_TOKEN_DOT) {
                    t = ctr_clex_tok();
                    if (t == CTR_TOKEN_DOT) {
                        t = ctr_clex_tok();
                        if (t == CTR_TOKEN_NUMBER) { // it's a range with a specific step
                            ctr_tnode* rs = re;
                            ne = ctr_clex_tok_value();
                            le = ctr_clex_tok_value_length();
                            re = ctr_cparse_create_node(CTR_AST_NODE);
                            re->value = ctr_heap_allocate_tracked(sizeof(char) * le);
                            re->type = CTR_AST_NODE_LTRNUM;
                            memcpy(re->value, ne, le);
                            re->vlen = le;

                            ctr_tnode* rv = ctr_cparse_create_generator_node_step(
                                /* from = */ r,
                                /* step = */ rs,
                                /* to   = */ re);
                            return rv;
                        } else if (t != CTR_TOKEN_FIN)
                            ctr_clex_putback();
                    } else if (t != CTR_TOKEN_FIN)
                        ctr_clex_putback();
                } else if (t != CTR_TOKEN_FIN)
                    ctr_clex_putback();
                ctr_tnode* rv = ctr_cparse_create_generator_node_simple(
                    /* from = */ r,
                    /* to   = */ re);
                return rv;
            } else if (t != CTR_TOKEN_FIN)
                ctr_clex_putback();
        } else if (t != CTR_TOKEN_FIN)
            ctr_clex_putback();
    }
    ctr_clex_putback();
    return r;
}

/**
 * CTRParserBooleanFalse
 *
 * Generates a node to represent a boolean False.
 */
ctr_tnode* ctr_cparse_false()
{
    ctr_tnode* r;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRBOOLFALSE;
    r->value = ctr_heap_allocate_tracked(sizeof(char) * 5);
    memcpy(r->value, "False", 5);
    r->vlen = 5;
    return r;
}

/**
 * CTRParserBooleanTrue
 *
 * Generates a node to represent a boolean True.
 */
ctr_tnode* ctr_cparse_true()
{
    ctr_tnode* r;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRBOOLTRUE;
    r->value = ctr_heap_allocate_tracked(sizeof(char) * 4);
    memcpy(r->value, "True", 4);
    r->vlen = 4;
    return r;
}

/**
 * CTRParserNil
 *
 * Generates a node to represent Nil
 */
ctr_tnode* ctr_cparse_nil()
{
    ctr_tnode* r;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRNIL;
    r->value = "Nil";
    r->vlen = 3;
    return r;
}

/**
 * CTRParserReceiver
 *
 * Generates a node to represent a receiver (of a message).
 */
ctr_tnode* ctr_cparse_receiver()
{
    int t;
    t = ctr_clex_tok();
    ctr_clex_putback();
    switch (t) {
    case CTR_TOKEN_NIL:
        return ctr_cparse_nil();
    case CTR_TOKEN_BOOLEANYES:
        return ctr_cparse_true();
    case CTR_TOKEN_BOOLEANNO:
        return ctr_cparse_false();
    case CTR_TOKEN_NUMBER:
        return ctr_cparse_number();
    case CTR_TOKEN_QUOTE:
        return ctr_cparse_string();
    case CTR_TOKEN_FANCY_QUOT_OPEN:
        return ctr_cparse_fancy_string();
    case CTR_TOKEN_REF:
        return ctr_cparse_ref();
    case CTR_TOKEN_BLOCKOPEN:
        return ctr_cparse_block();
    case CTR_TOKEN_BLOCKOPEN_MAP: {
        ctr_tnode* t = ctr_cparse_block_capture();
        t->lexical = 1;
        t->nodes->next->node->lexical = 1;
        return t;
    }
    case CTR_TOKEN_PAROPEN:
        return ctr_cparse_popen();
    case CTR_TOKEN_TUPOPEN:
        return ctr_cparse_tuple(CTR_TOKEN_TUPCLOSE);
    case CTR_TOKEN_LITERAL_ESC:
        return ctr_cparse_lit_esc(CTR_TOKEN_TUPCLOSE);
    case CTR_TOKEN_SYMBOL:
        return ctr_cparse_symbol();
    default:
        /* This function always exits, so return a dummy value. */
        ctr_cparse_emit_error_unexpected(t, "Expected a message recipient.\n");
        return speculative_parse ? ctr_fake_parse_hole() : NULL;
    }
}

/**
 * CTRParserAssignment
 *
 * Generates a node to represent an assignment.
 */
ctr_tnode* ctr_cparse_assignment(ctr_tnode* r)
{
    ctr_tnode* a;
    ctr_tlistitem* li;
    ctr_tlistitem* liAssignExpr;
    ctr_clex_tok();
    a = ctr_cparse_create_node(CTR_AST_NODE);
    li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    liAssignExpr = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    a->type = CTR_AST_NODE_EXPRASSIGNMENT;
    a->nodes = li;
    li->node = r;
    liAssignExpr->node = ctr_cparse_expr(0);
    li->next = liAssignExpr;
    return a;
}

/**
 * CTRParserExpression
 *
 * Generates a set of nodes to represent an expression.
 */
const char* me_s = "me";
ctr_tnode* ctr_cparse_expr(int mode)
{
    ctr_tnode* r;
    ctr_tnode* e;
    int t2;
    ctr_tlistitem* nodes;
    ctr_tlistitem* rli;
    struct lexer_state lexer_state;
    ctr_clex_dump_state(&lexer_state);
    r = ctr_cparse_receiver();
    t2 = ctr_clex_tok();
    ctr_clex_putback();

    if (mode == -1)
        return r;

    /* user tries to put colon directly after recipient */
    if (t2 == CTR_TOKEN_COLON) {
        /* Parse as if we had a "me" before this */
        ctr_clex_load_state(lexer_state);
        if (ctr_clex_inject_token(CTR_TOKEN_REF, me_s, 2, 2))
            ctr_cparse_emit_error_unexpected(
                t2, "Recipient cannot be followed by a colon in this state.\n");
        return ctr_cparse_expr(mode);
    }

    if (t2 == CTR_TOKEN_ASSIGNMENT) {
        // if ( r->type != CTR_AST_NODE_REFERENCE ) {
        //     ctr_cparse_emit_error_unexpected( t2, "Invalid left-hand
        //     assignment.\n" ); exit(1);
        // }
        e = ctr_cparse_assignment(r);
    } else if (t2 == CTR_TOKEN_PASSIGNMENT) {
        if (r->type == CTR_AST_NODE_REFERENCE) // it's an alias for my REF is EXPR
            r->modifier = 1;                   // set private */
        e = ctr_cparse_assignment(r);          // go as usual
                                               /*r->nodes->node->type = CTR_AST_NODE_LTRSTRING;
                                                  e = ctr_cparse_create_node(CTR_AST_NODE);
                                                  e->type = CTR_AST_NODE_EXPRMESSAGE;
                                                  e->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
                                                  e->nodes->node = ctr_cparse_create_node(CTR_AST_NODE);
                                                  e->nodes->node->value = ctr_heap_allocate(sizeof(char)*7); //Reflect
                                                  memcpy(e->nodes->node->value, "Reflect", 7);
                                                  e->nodes->node->vlen = 7;
                                                  e->nodes->node->type = CTR_AST_NODE_REFERENCE;
                                                  e->nodes->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
                                                  e->nodes->next->node = ctr_cparse_create_node(CTR_AST_NODE);
                                                  e->nodes->next->node->type = CTR_AST_NODE_KWMESSAGE;
                                                  e->nodes->next->node->value = ctr_heap_allocate(sizeof(char)*7);
                                                  //set:to: memcpy(e->nodes->next->node->value, "set:to:", 7);
                                                  e->nodes->next->node->vlen = 7;
                                                  e->nodes->next->node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
                                                  e->nodes->next->node->nodes->node = r->nodes->node;
                                                  e->nodes->next->node->nodes->next =
                                                  ctr_heap_allocate(sizeof(ctr_tlistitem));
                                                  e->nodes->next->node->nodes->next->node = r->nodes->next->node; */
    } else if (t2 != CTR_TOKEN_DOT && t2 != CTR_TOKEN_PARCLOSE && t2 != CTR_TOKEN_CHAIN && t2 != CTR_TOKEN_TUPCLOSE && t2 != CTR_TOKEN_FANCY_QUOT_CLOS) {
        e = ctr_cparse_create_node(CTR_AST_NODE);
        e->type = CTR_AST_NODE_EXPRMESSAGE;
        nodes = ctr_cparse_messages(r, mode);
        int tv = ctr_clex_tok();
        int vtv = 0;
        while (tv == CTR_TOKEN_INV) {
            // arg0 `callee` arg1
            //^    ^tv
            //`- receiver
            ctr_clex_tok();
            ctr_tnode* receiver = ctr_cparse_create_node(CTR_AST_NODE);
            receiver->type = CTR_AST_NODE_REFERENCE;
            receiver->vlen = ctr_clex_tok_value_length() - 1;
            receiver->value = ctr_heap_allocate_tracked(receiver->vlen);
            memcpy(receiver->value, ctr_clex_tok_value(), receiver->vlen);
            fixity_lookup_rv fix = ctr_lookup_fix(receiver->value, receiver->vlen);
            int mlen = fix.prec == 2 ? 10 : 9;
            const char* mmsg = fix.prec == 2 ? "_ApplyAll:" : "applyAll:";
            char* msg = ctr_heap_allocate_tracked(sizeof(char) * mlen);
            memcpy(msg, mmsg, mlen);
            int length = mlen;
            ctr_tlistitem* li = ctr_heap_allocate_tracked(sizeof(*li));
            li->node = ctr_heap_allocate_tracked(sizeof(*(li->node)));
            li->node->type = CTR_AST_NODE_IMMUTABLE;
            li->node->nodes = ctr_heap_allocate_tracked(sizeof(*li));
            ctr_tlistitem* ll = li->node->nodes;
            ll->node = ctr_heap_allocate_tracked(sizeof(*ll->node));
            ll->node->nodes = ctr_heap_allocate_tracked(sizeof(*ll));
            ll = ll->node->nodes;
            ll->node = r;
            char texpr_res = ctr_transform_template_expr;
            if (fix.lazy) {
                ll->node = ctr_cparse_create_node(CTR_AST_NODE);
                ll->node->type = CTR_AST_NODE_RAW;
                ll->node->modifier = 1;
                ll->node->nodes = ctr_heap_allocate(sizeof(*ll));
                ll->node->nodes->node = r;
            }
            ll->next = ctr_heap_allocate_tracked(sizeof(*ll));
            if (fix.lazy) {
                int t = ctr_clex_tok();
                ctr_clex_putback();
                if (t == CTR_TOKEN_TUPOPEN) {
                    ctr_transform_template_expr = 2;
                } else {
                    ctr_transform_template_expr = 1;
                }
            }
            ll->next->node = ctr_cparse_expr(fix.fix + fix.prec); // get next argument
            if (ctr_transform_template_expr == 1) {
                ctr_tnode* rv = ll->next->node;
                ll->next->node = ctr_cparse_create_node(CTR_AST_NODE);
                ll->next->node->type = CTR_AST_NODE_RAW;
                ll->next->node->modifier = 1;
                ll->next->node->nodes = ctr_heap_allocate(sizeof(*rv->nodes));
                ll->next->node->nodes->node = rv;
            }
            ctr_transform_template_expr = texpr_res;
            // arguments in li
            ctr_tlistitem* rli = ctr_heap_allocate_tracked(sizeof(*rli));
            rli->node = ctr_cparse_create_node(CTR_AST_NODE);
            rli->node->type = CTR_AST_NODE_KWMESSAGE;
            rli->node->value = msg;
            rli->node->vlen = length;
            rli->node->nodes = li;
            r = ctr_cparse_create_node(CTR_AST_NODE);
            r->type = CTR_AST_NODE_EXPRMESSAGE;
            r->nodes = ctr_heap_allocate_tracked(sizeof(*rli));
            r->nodes->node = receiver;
            r->nodes->next = rli;
            tv = ctr_clex_tok();
        }
        ctr_clex_putback();
        if (vtv)
            return r;
        if (nodes == NULL) {
            return r; /* no messages, then just return receiver (might be in case of
                         argument). */
        }
        rli = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
        rli->node = r;
        rli->next = nodes;
        e->nodes = rli;
    } else {
        return r;
    }
    return e;
}

/**
 * CTRParserReturn
 *
 * Generates a node to represent a return from a block of code.
 */
ctr_tnode* ctr_cparse_ret()
{
    ctr_tlistitem* li;
    ctr_tnode* r;
    ctr_clex_tok();
    r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_RETURNFROMBLOCK;
    li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    r->nodes = li;
    li->node = ctr_cparse_expr(0);
    return r;
}

/**
 * CTRParserFin
 *
 * Generates a node to represent the end of a program.
 */
ctr_tnode* ctr_cparse_fin()
{
    callShorthand->value = CTR_TOKEN_TUPOPEN;
    callShorthand->value_e = CTR_TOKEN_TUPCLOSE;
    extensionsPra->value = 0;
    // clear_fixity_map();
    ctr_tnode* f;
    ctr_clex_tok();
    f = ctr_cparse_create_node(CTR_AST_NODE);
    f->type = CTR_AST_NODE_ENDOFPROGRAM;
    return f;
}

/**
 * CTRParserPure
 *
 * Generates an empty node for a pure block, and executes the block
 */
ctr_tnode* ctr_cparse_pure()
{
    char* code_s = ctr_code;
    ctr_clex_putback();
    char* end = ctr_clex_scan_balanced('}', '{');
    if (end) {
        char* code = ctr_heap_allocate(end - code_s + 1);
        memcpy(code, code_s, end - code_s);
        code[end - code_s] = 0;
        printf("Pure dialect block ignored\n");
        ctr_clex_tok();
    }
    ctr_tnode* r = ctr_cparse_create_node(CTR_AST_NODE);
    r->type = CTR_AST_NODE_LTRNIL;
    r->value = "Nil";
    r->vlen = 3;
    return r;
}

ctr_tnode* ctr_cparse_comptime()
{
    char ret = 0;
    int discard = 0;
    int t = ctr_clex_tok();
    if (t == CTR_TOKEN_TUPOPEN) {
        while ((t = ctr_clex_tok()) != CTR_TOKEN_TUPCLOSE) {
            if (t == CTR_TOKEN_REF) {
                if (ctr_clex_tok_value_length() == 7 && strncmp(ctr_clex_tok_value(), "discard", 7) == 0)
                    discard = 1;
                else
                    printf("@comptime warning: unknown attribute %.*s ignored\n",
                        ctr_clex_tok_value_length(), ctr_clex_tok_value());
            } else {
                printf("@comptime warning: invalid attribute type %d ignored\n", t);
            }
        }
    } else
        ctr_clex_putback();
    ctr_tnode* node = ctr_cparse_expr(0);
    // printf("@comptime node (%p):\n", node);
    // ctr_internal_debug_tree(node, 1);
    ctr_object* val = ctr_cwlk_expr(node, &ret);
    if (discard) {
        ctr_clex_putback();
        return ctr_cparse_nil();
    }
    if (!ctr_ast_is_splice(val)) {
        ctr_cparse_emit_error_unexpected(
            CTR_TOKEN_INV,
            "Expected an AST splice as a return value from a comptime expression");
        return speculative_parse ? ctr_fake_parse_hole() : NULL;
    }
    if (!val->value.rvalue || !val->value.rvalue->ptr) {
        ctr_cparse_emit_error_unexpected(CTR_TOKEN_INV,
            "Expected a valid AST splice as a return "
            "value from a comptime expression");
        return speculative_parse ? ctr_fake_parse_hole() : NULL;
    }
    return val->value.rvalue->ptr;
}

/**
 * CTRParserStatement
 *
 * Generates a set of nodes representing a statement.
 */
ctr_tlistitem* ctr_cparse_statement()
{
    ctr_tlistitem* li = (ctr_tlistitem*)ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    int t = ctr_clex_tok();
    ctr_clex_putback();
    if (t == CTR_TOKEN_FIN) {
        li->node = ctr_cparse_fin();
        return li;
    } else if (t == CTR_TOKEN_RET) {
        li->node = ctr_cparse_ret();
    } else {
        li->node = ctr_cparse_expr(0);
    }
    t = ctr_clex_tok();
    if (t != CTR_TOKEN_DOT) {
        if (t == CTR_TOKEN_QUOTE || t == CTR_TOKEN_FANCY_QUOT_OPEN) {
            ctr_cparse_emit_error_unexpected(t, "Expected a closing quote.\n");
            if (speculative_parse)
                if (ctr_clex_inject_token(
                        t == CTR_TOKEN_QUOTE ? t : CTR_TOKEN_FANCY_QUOT_CLOS, "'", 1,
                        1)) {
                    ctr_cparse_emit_error_unexpected(
                        t, "Speculative parsing failed, not enough vector space\n");
                    return NULL;
                }
        } else {
            if (t != CTR_TOKEN_FIN) {
                ctr_cparse_emit_error_unexpected(t, "Expected a dot (.).\n");
                if (speculative_parse) {
                    ctr_clex_putback();
                    if (ctr_clex_inject_token(CTR_TOKEN_DOT, ".", 1, 1)) {
                        ctr_cparse_emit_error_unexpected(
                            t, "Speculative parsing failed, not enough vector space\n");
                        return NULL;
                    }
                }
            }
        }
        if (!li->node)
            li->node = ctr_fake_parse_hole();
        // li->node = ctr_cparse_fin();
    }
    return li;
}

/**
 * CTRParserProgram
 *
 * Generates the nodes to represent the entire program
 * as an Abstract Syntax Tree (AST).
 */
ctr_tnode* ctr_cparse_program()
{
    ctr_tnode* program = ctr_cparse_create_node(CTR_AST_PROGRAM);
    ctr_tlistitem* pli;
    int first = 1;
    while (1) {
        ctr_tlistitem* li = ctr_cparse_statement();
        if (first) {
            first = 0;
            program->nodes = li;
        } else {
            pli->next = li;
        }
        if (li->node == NULL || li->node->type == CTR_AST_NODE_ENDOFPROGRAM) {
            li->node = ctr_cparse_fin();
            break;
        }
        pli = li;
    }
    return program;
}

/**
 * CTRParserStart
 *
 * Begins the parsing stage of a program.
 */
ctr_tnode* ctr_cparse_parse(char* prg, char* pathString)
{
    ctr_tnode* program;
    ctr_clex_load(prg);
    char* oldp = ctr_cparse_current_program;
    ctr_cparse_current_program = pathString;
    program = ctr_cparse_program();
    program->value = pathString;
    program->vlen = strlen(pathString);
    program->type = CTR_AST_NODE_PROGRAM;
    ctr_cparse_current_program = oldp;
    return program;
}

ctr_tnode* ctr_deep_copy_ast(ctr_tnode* src)
{
    if (!src)
        return src;

    ctr_tnode* dst = ctr_heap_allocate(sizeof *dst);
    dst->vlen = src->vlen;
    if (src->value)
        dst->value = src->value; // TODO: figure out copying this
    dst->type = src->type;
    dst->modifier = src->modifier;
    dst->lexical = src->lexical;

    if (src->type == CTR_AST_NODE_EMBED) {
        if (src->modifier == 2) {
            ctr_tlistitem* nodes = ctr_heap_allocate(sizeof *nodes);
            nodes->next = ctr_heap_allocate(sizeof *nodes);
            nodes->node = src->nodes->node;
            nodes->next->node = src->nodes->next->node;
            dst->nodes = nodes;
            return dst;
        } else if (src->modifier == 1) {
            ctr_tlistitem* nodes = ctr_heap_allocate(sizeof *nodes);
            nodes->node = src->nodes->node;
            dst->nodes = nodes;
            return dst;
        }
    }
    ctr_tlistitem* nodes = src->nodes;
    dst->nodes = NULL;
    if (nodes) {
        ctr_tlistitem *dnodes = ctr_heap_allocate(sizeof *nodes), *pnodes = NULL;
        dst->nodes = dnodes;
        while (nodes) {
            dnodes->node = ctr_deep_copy_ast(nodes->node);
            dnodes->next = ctr_heap_allocate(sizeof *dnodes);
            pnodes = dnodes;
            dnodes = dnodes->next;
            nodes = nodes->next;
        }
        ctr_heap_free(dnodes);
        pnodes->next = NULL;
    }
    return dst;
}
