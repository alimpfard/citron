#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "citron.h"
#include "symbol.h"

char *ctr_cparse_current_program;
int do_compare_locals = 0;
int all_plains_private = 0;
extern int ctr_cwlk_replace_refs;
int ctr_paramlist_has_name(char* namenode, size_t len) {
	if(!ctr_cparse_calltime_names || len==0) return 0;
	else {
		ctr_tlistitem* name = ctr_cparse_calltime_names->nodes;
		while(name) {
			int vararg = name->node->value[0] == '*';
			if(unlikely(name->node->vlen == len || vararg)) {
				if(strncmp(name->node->value+vararg, namenode, len-vararg) == 0) return 1;
			}
			name = name->next;
		}
		return 0;
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
void ctr_cparse_emit_error_unexpected(int t, char *hint)
{
	char buf[1024];
	char *message = ctr_clex_tok_describe(t);
	sprintf(buf, "Parse error, unexpected %s ( %s: %d )\n", message,
		ctr_cparse_current_program, ctr_clex_line_number + 1);
	if (ctr_cparse_quiet)
		return;
	//memcpy(ctr_last_parser_error, buf, strlen(buf));
	printf("%s", buf);
	if (hint) {
		printf("%s", hint);
	}
#ifdef EXIT_ON_ERROR
	exit(1);
#endif
}

/**
 * CTRParserCreateNode
 *
 * Creates a parser node and adds it to the source map.
 */
ctr_tnode *ctr_cparse_create_node(int type)
{
	ctr_tnode *node =
	    (ctr_tnode *) ctr_heap_allocate_tracked(sizeof(ctr_tnode));
	if (ctr_source_mapping) {
		ctr_source_map *m =
		    (ctr_source_map *)
		    ctr_heap_allocate_tracked(sizeof(ctr_source_map));
		m->line = ctr_clex_line_number;
		m->node = node;
		if (ctr_source_map_head) {
			m->next = ctr_source_map_head;
			ctr_source_map_head = m;
		} else {
			ctr_source_map_head = m;
		}
	}
	return node;
}

/**
 * CTRParserMessage
 *
 * Creates the AST nodes for sending a message.
 *
 * - precedence mode 0: no argument (allows processing of unary message, binary message and keyword message)
 * - precedence mode 1: as argument of keyword message (allows processing of unary message and binary message)
 * - precedence mode 2: as argument of binary message (only allows processing of unary message)
 */
ctr_tnode *ctr_cparse_message(int mode)
{
	long msgpartlen;	/* length of part of message string */
	ctr_tnode *m;
	int t;
	char *s;
	char *msg;
	ctr_tlistitem *li;
	ctr_tlistitem *curlistitem;
	int lookAhead;
	int isBin;
	int first;
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
	isBin = ctr_utf8_is_one_cluster(msg, msgpartlen);
	if (mode == 2 && isBin) {
		ctr_clex_putback();
		return m;
	}
	if (isBin) {
		m->type = CTR_AST_NODE_BINMESSAGE;
		m->value = msg;
		m->vlen = msgpartlen;
		li = (ctr_tlistitem *)
		    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
		li->node = ctr_cparse_expr(2);
		m->nodes = li;
		return m;
	}
	lookAhead = ctr_clex_tok();
	ctr_clex_putback();
	if (lookAhead == CTR_TOKEN_COLON) {
		if (mode > 0) {
			ctr_clex_putback();
			return m;
		}
		*(msg + msgpartlen) = ':';
		msgpartlen += 1;
		if ((msgpartlen) > 255) {
			ctr_cparse_emit_error_unexpected(t,
							 "Message too long\n");
		}
		m->type = CTR_AST_NODE_KWMESSAGE;
		t = ctr_clex_tok();
		first = 1;
		while (1) {
			li = (ctr_tlistitem *)
			    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
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
			if (t == CTR_TOKEN_REF) {
				long l = ctr_clex_tok_value_length();
				if ((msgpartlen + l) > 255) {
					ctr_cparse_emit_error_unexpected(t,
									 "Message too long\n");
				}
				memcpy((msg + msgpartlen), ctr_clex_tok_value(),
				       l);
				msgpartlen = msgpartlen + l;
				*(msg + msgpartlen) = ':';
				msgpartlen++;
				t = ctr_clex_tok();
				if (t != CTR_TOKEN_COLON) {
					ctr_cparse_emit_error_unexpected(t,
									 "Expected colon.\n");
				}
			}
		}
		ctr_clex_putback();	/* not a colon so put back */
		m->value = msg;
		m->vlen = msgpartlen;
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
ctr_tlistitem *ctr_cparse_messages(ctr_tnode * r, int mode)
{
	int t = ctr_clex_tok();
	ctr_tlistitem *pli = NULL;
	ctr_tlistitem *li = NULL;
	ctr_tlistitem *fli = NULL;
	int first = 1;
	ctr_tnode *node = NULL;
	/* explicit chaining (,) only allowed for keyword message: Console write: 3 factorial, write: 3 factorial is not possible otherwise. */
	while ((t == CTR_TOKEN_REF
		|| (t == CTR_TOKEN_CHAIN && node
		    && node->type == CTR_AST_NODE_KWMESSAGE))) {
		if (t == CTR_TOKEN_CHAIN) {
			t = ctr_clex_tok();
			if (t != CTR_TOKEN_REF) {
				ctr_cparse_emit_error_unexpected(t,
								 "Expected message.\n");
			}
		}
		li = (ctr_tlistitem *)
		    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
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
 * CTRParserTuple
 *
 * Generates a node to represent an immutable array (tuple)
 */
ctr_tnode *ctr_cparse_tuple()
{
	ctr_tnode *r;
	ctr_tlistitem *codeBlockPart1;
	ctr_tnode *paramList;
	ctr_tlistitem *previousListItem;
	int t;
	ctr_clex_tok();		//eat the [
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_IMMUTABLE;
	codeBlockPart1 =
	    (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	r->nodes = codeBlockPart1;
	paramList = ctr_cparse_create_node(CTR_AST_NODE);
	codeBlockPart1->node = paramList;
	paramList->type = CTR_AST_NODE_NESTED;

	t = ctr_clex_tok();
	ctr_clex_putback();
	if (t == CTR_TOKEN_TUPCLOSE) {
		ctr_clex_tok();	//eat the ending ]
		return r;
	} else {
		ctr_tlistitem *paramListItem = (ctr_tlistitem *)
		    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
		paramList->nodes = paramListItem;
		paramListItem->node = ctr_cparse_expr(1);
		previousListItem = paramListItem;
	}
	while ((t = ctr_clex_tok()) == CTR_TOKEN_CHAIN) {
		/* okay we have new parameter, load it */
		ctr_tlistitem *paramListItem = (ctr_tlistitem *)
		    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
		paramListItem->node = ctr_cparse_expr(1);
		previousListItem->next = paramListItem;
		previousListItem = paramListItem;
	}
	if (t != CTR_TOKEN_TUPCLOSE)
		ctr_cparse_emit_error_unexpected(t, "Expected ].");
	return r;
}

/**
 * CTRParserSymbol
 *
 * Generates a symbol or pulls it from the static symbol list.
 */
ctr_tnode *ctr_cparse_symbol()
{
	ctr_tnode *r;
	ctr_tlistitem *li;
	r = ctr_cparse_create_node(CTR_AST_NODE);
	int t = ctr_clex_tok();
	if (t != CTR_TOKEN_SYMBOL) return NULL;
	ctr_clex_tok();
	r->type = CTR_AST_NODE_SYMBOL;
	li = (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	ctr_size vlen = ctr_clex_tok_value_length();
	r->value = (char*)ctr_get_or_create_symbol_table_entry(ctr_clex_tok_value(), vlen);
	r->vlen = -1; //sig
	return r;
}

/**
 * CTRParserPOpen
 *
 * Generates a set of nested nodes.
 */
ctr_tnode *ctr_cparse_popen()
{
	ctr_tnode *r;
	ctr_tlistitem *li;
	int t;
	ctr_clex_tok();
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_NESTED;
	li = (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	r->nodes = li;
	li->node = ctr_cparse_expr(0);
	t = ctr_clex_tok();
	if (t != CTR_TOKEN_PARCLOSE) {
		ctr_cparse_emit_error_unexpected(t, "Expected ).\n");
	}
	return r;
}

/**
 * CTRParserBlock
 *
 * Generates a set of AST nodes to represent a block of code.
 */
ctr_tnode *ctr_cparse_block_(int autocap);
// __attribute__ ((always_inline))
ctr_tnode *ctr_cparse_block()
{
	return ctr_cparse_block_(0);
}

// __attribute__ ((always_inline))
ctr_tnode *ctr_cparse_block_capture()
{
	return ctr_cparse_block_(1);
}

ctr_tnode *ctr_cparse_block_(int autocap)
{
	ctr_tnode *r;
	ctr_tlistitem *codeBlockPart1;
	ctr_tlistitem *codeBlockPart2;
	ctr_tnode *paramList;
	ctr_tnode *codeList;
	ctr_tlistitem *previousListItem;
	ctr_tlistitem *previousCodeListItem;
	int t;
	int first;
	ctr_clex_tok();
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_CODEBLOCK;
	codeBlockPart1 =
	    (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	r->nodes = codeBlockPart1;
	codeBlockPart2 =
	    (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	r->nodes->next = codeBlockPart2;
	paramList = ctr_cparse_create_node(CTR_AST_NODE);
	codeList = ctr_cparse_create_node(CTR_AST_NODE);
	codeBlockPart1->node = paramList;
	codeBlockPart2->node = codeList;
	paramList->type = CTR_AST_NODE_PARAMLIST;
	codeList->type = CTR_AST_NODE_INSTRLIST;
	t = ctr_clex_tok();
	first = 1;
	while (t == CTR_TOKEN_COLON) {
		/* okay we have new parameter, load it */
		t = ctr_clex_tok();
		ctr_tlistitem *paramListItem = (ctr_tlistitem *)
		    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
		ctr_tnode *paramItem = ctr_cparse_create_node(CTR_AST_NODE);
		long l = ctr_clex_tok_value_length();
		paramItem->value = ctr_heap_allocate_tracked(sizeof(char) * l);
		memcpy(paramItem->value, ctr_clex_tok_value(), l);
		paramItem->vlen = l;
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
	int oldallpl = all_plains_private;
	int olddcl = do_compare_locals;
	do_compare_locals = autocap;
	ctr_tnode* oldcalltime = ctr_cparse_calltime_names;
	ctr_cparse_calltime_names = paramList;
	all_plains_private = autocap;
	if (ctr_transform_lambda_shorthand) {
		ctr_transform_lambda_shorthand = 0;
		ctr_tlistitem *codeListItem;
		ctr_tnode *codeNode;
		ctr_clex_putback();
		codeListItem =
				(ctr_tlistitem *)
				ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
		codeNode = ctr_cparse_create_node(CTR_AST_NODE);
		codeNode = ctr_cparse_expr(0); //parse a single expression
		codeListItem->node = codeNode;
		codeList->nodes = codeListItem;
		previousCodeListItem = codeListItem;
	}
	else {
		while ((first || t == CTR_TOKEN_DOT)) {
			ctr_tlistitem *codeListItem;
			ctr_tnode *codeNode;
			if (first) {
				ctr_clex_putback();
			}
			t = ctr_clex_tok();
			if (t == CTR_TOKEN_BLOCKCLOSE)
				break;
			ctr_clex_putback();
			codeListItem =
			    (ctr_tlistitem *)
			    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
			codeNode = ctr_cparse_create_node(CTR_AST_NODE);
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
				ctr_cparse_emit_error_unexpected(t,
								 "Expected a dot (.).\n");
			}
		}
	}
	all_plains_private = oldallpl;
	do_compare_locals=olddcl;
	ctr_cparse_calltime_names = oldcalltime;
	r->modifier = /*CTR_MODIFIER_AUTOCAPTURE */ autocap == 1;
	return r;
}

/**
 * CTRParserReference
 *
 * Generates the nodes to respresent a variable or property.
 */
ctr_tnode *ctr_cparse_ref()
{
	ctr_tnode *r;
	char *tmp;
	ctr_clex_tok();
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_REFERENCE;
	r->vlen = ctr_clex_tok_value_length();
	tmp = ctr_clex_tok_value();
	if (all_plains_private)
		r->modifier = (do_compare_locals) ? (ctr_paramlist_has_name(tmp, r->vlen) ? 0 : 3) : 3;
	if (strncmp(ctr_clex_keyword_my, tmp, ctr_clex_keyword_my_len) == 0
	    && r->vlen == ctr_clex_keyword_my_len) {
		int t = ctr_clex_tok();
		if (t != CTR_TOKEN_REF) {
			ctr_cparse_emit_error_unexpected(t,
							 "'My' should always be followed by a property name!\n");
		}
		tmp = ctr_clex_tok_value();
		r->modifier = 1;
		r->vlen = ctr_clex_tok_value_length();
	}
	if (strncmp(ctr_clex_keyword_var, tmp, ctr_clex_keyword_var_len) == 0
	    && r->vlen == ctr_clex_keyword_var_len) {
		int t = ctr_clex_tok();
		if (t != CTR_TOKEN_REF) {
			ctr_cparse_emit_error_unexpected(t,
							 "Keyword 'var' should always be followed by property name!\n");
		}
		tmp = ctr_clex_tok_value();
		r->modifier = 2;
		r->vlen = ctr_clex_tok_value_length();
	}
	if (strncmp(ctr_clex_keyword_const, tmp, ctr_clex_keyword_const_len) ==
	    0 && r->vlen == ctr_clex_keyword_const_len) {
		int t = ctr_clex_tok();
		if (t != CTR_TOKEN_REF) {
			ctr_cparse_emit_error_unexpected(t,
							 "'const' must always be followed by a single reference/property\n");
		}
		tmp = ctr_clex_tok_value();
		r->modifier = 3;
		r->vlen = ctr_clex_tok_value_length();
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
ctr_tnode *ctr_cparse_string()
{
	ctr_tnode *r;
	char *n;
	ctr_size vlen;
	ctr_clex_tok();
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_LTRSTRING;
	n = ctr_clex_readstr();
	vlen = ctr_clex_tok_value_length();
	r->value = ctr_heap_allocate_tracked(sizeof(char) * vlen);
	memcpy(r->value, n, vlen);
	r->vlen = vlen;
	ctr_clex_tok();		/* eat trailing quote. */
	return r;
}

/**
 * CTRParserNumber
 *
 * Generates a node to represent a number.
 */
void ctr_internal_debug_tree(ctr_tnode * ti, int indent);
ctr_tnode *ctr_cparse_number()
{
	char *n;
	ctr_tnode *r;
	long l;
	ctr_clex_tok();
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_LTRNUM;
	n = ctr_clex_tok_value();
	l = ctr_clex_tok_value_length();
	r->value = ctr_heap_allocate_tracked(sizeof(char) * l);
	memcpy(r->value, n, l);
	r->vlen = l;
	return r;
}

/**
 * CTRParserBooleanFalse
 *
 * Generates a node to represent a boolean False.
 */
ctr_tnode *ctr_cparse_false()
{
	ctr_tnode *r;
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
ctr_tnode *ctr_cparse_true()
{
	ctr_tnode *r;
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
ctr_tnode *ctr_cparse_nil()
{
	ctr_tnode *r;
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
ctr_tnode *ctr_cparse_receiver()
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
	case CTR_TOKEN_REF:
		return ctr_cparse_ref();
	case CTR_TOKEN_BLOCKOPEN:
		return ctr_cparse_block();
	case CTR_TOKEN_BLOCKOPEN_MAP:
		{
			ctr_tnode* t = ctr_cparse_block_capture();
			t->lexical = 1;
			t->nodes->next->node->lexical = 1;
			return t;
	}
	case CTR_TOKEN_PAROPEN:
		return ctr_cparse_popen();
	case CTR_TOKEN_TUPOPEN:
		return ctr_cparse_tuple();
	case CTR_TOKEN_SYMBOL:
		return ctr_cparse_symbol();
	default:
		/* This function always exits, so return a dummy value. */
		ctr_cparse_emit_error_unexpected(t,
						 "Expected a message recipient.\n");
		return NULL;
	}
}

/**
 * CTRParserAssignment
 *
 * Generates a node to represent an assignment.
 */
ctr_tnode *ctr_cparse_assignment(ctr_tnode * r)
{
	ctr_tnode *a;
	ctr_tlistitem *li;
	ctr_tlistitem *liAssignExpr;
	ctr_clex_tok();
	a = ctr_cparse_create_node(CTR_AST_NODE);
	li = (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	liAssignExpr =
	    (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
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
ctr_tnode *ctr_cparse_expr(int mode)
{
	ctr_tnode *r;
	ctr_tnode *e;
	int t2;
	ctr_tlistitem *nodes;
	ctr_tlistitem *rli;
	r = ctr_cparse_receiver();
	t2 = ctr_clex_tok();
	ctr_clex_putback();

	/* user tries to put colon directly after recipient */
	if (t2 == CTR_TOKEN_COLON) {
		ctr_cparse_emit_error_unexpected(t2,
						 "Recipient cannot be followed by a colon.\n");
	}

	if (t2 == CTR_TOKEN_ASSIGNMENT) {
		// if ( r->type != CTR_AST_NODE_REFERENCE ) {
		//     ctr_cparse_emit_error_unexpected( t2, "Invalid left-hand assignment.\n" );
		//     exit(1);
		// }
		e = ctr_cparse_assignment(r);
	} else if (t2 == CTR_TOKEN_PASSIGNMENT) {
		if (r->type != CTR_AST_NODE_REFERENCE) {
			ctr_cparse_emit_error_unexpected(t2,
							 "Invalid left-hand assignment.\n");
			exit(1);
		}
		r->modifier = 1;	//set private */
		e = ctr_cparse_assignment(r);	//go as usual
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
		e->nodes->next->node->value = ctr_heap_allocate(sizeof(char)*7); //set:to:
		memcpy(e->nodes->next->node->value, "set:to:", 7);
		e->nodes->next->node->vlen = 7;
		e->nodes->next->node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
		e->nodes->next->node->nodes->node = r->nodes->node;
		e->nodes->next->node->nodes->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
		e->nodes->next->node->nodes->next->node = r->nodes->next->node;*/
	} else if (t2 != CTR_TOKEN_DOT &&
		   t2 != CTR_TOKEN_PARCLOSE && (t2 != CTR_TOKEN_CHAIN
						&& mode !=
						CTR_AST_NODE_IMMUTABLE)) {
		e = ctr_cparse_create_node(CTR_AST_NODE);
		e->type = CTR_AST_NODE_EXPRMESSAGE;
		nodes =
		    ctr_cparse_messages(r,
					mode ==
					CTR_AST_NODE_IMMUTABLE ? 0 : mode);
		if (nodes == NULL) {
			ctr_clex_tok();
			ctr_clex_putback();
			return r;	/* no messages, then just return receiver (might be in case of argument). */
		}
		rli =
		    (ctr_tlistitem *)
		    ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
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
ctr_tnode *ctr_cparse_ret()
{
	ctr_tlistitem *li;
	ctr_tnode *r;
	ctr_clex_tok();
	r = ctr_cparse_create_node(CTR_AST_NODE);
	r->type = CTR_AST_NODE_RETURNFROMBLOCK;
	li = (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
	r->nodes = li;
	li->node = ctr_cparse_expr(0);
	return r;
}

/**
 * CTRParserFin
 *
 * Generates a node to represent the end of a program.
 */
ctr_tnode *ctr_cparse_fin()
{
	ctr_tnode *f;
	ctr_clex_tok();
	f = ctr_cparse_create_node(CTR_AST_NODE);
	f->type = CTR_AST_NODE_ENDOFPROGRAM;
	return f;
}

/**
 * CTRParserStatement
 *
 * Generates a set of nodes representing a statement.
 */
ctr_tlistitem *ctr_cparse_statement()
{
	ctr_tlistitem *li =
	    (ctr_tlistitem *) ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
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
		ctr_cparse_emit_error_unexpected(t, "Expected a dot (.).\n");
		li->node = ctr_cparse_fin();
	}
	return li;
}

/**
 * CTRParserProgram
 *
 * Generates the nodes to represent the entire program
 * as an Abstract Syntax Tree (AST).
 */
ctr_tnode *ctr_cparse_program()
{
	ctr_tnode *program = ctr_cparse_create_node(CTR_AST_PROGRAM);
	ctr_tlistitem *pli;
	int first = 1;
	while (1) {
		ctr_tlistitem *li = ctr_cparse_statement();
		if (first) {
			first = 0;
			program->nodes = li;
		} else {
			pli->next = li;
		}
		if (li->node == NULL
		    || li->node->type == CTR_AST_NODE_ENDOFPROGRAM) {
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
ctr_tnode *ctr_cparse_parse(char *prg, char *pathString)
{
	ctr_tnode *program;
	ctr_clex_load(prg);
	char *oldp = ctr_cparse_current_program;
	ctr_cparse_current_program = pathString;
	program = ctr_cparse_program();
	program->value = pathString;
	program->vlen = strlen(pathString);
	program->type = CTR_AST_NODE_PROGRAM;
	ctr_cparse_current_program = oldp;
	return program;
}
