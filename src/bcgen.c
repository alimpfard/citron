#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "citron.h"
#include "bytecode.h"
#include "khash.h"

KHASH_MAP_INIT_INT64(ast2proto, ctr_proto*)
static khash_t(ast2proto)* g_proto_registry = NULL;

static void registry_ensure(void)
{
    if (!g_proto_registry)
        g_proto_registry = kh_init(ast2proto);
}

ctr_proto* ctr_proto_for_ast(struct ctr_tnode* ast)
{
    if (!g_proto_registry || !ast)
        return NULL;
    khiter_t k = kh_get(ast2proto, g_proto_registry, (uint64_t)(uintptr_t)ast);
    if (k == kh_end(g_proto_registry))
        return NULL;
    return kh_val(g_proto_registry, k);
}

void ctr_proto_register(struct ctr_tnode* ast, ctr_proto* p)
{
    registry_ensure();
    int ret;
    khiter_t k = kh_put(ast2proto, g_proto_registry, (uint64_t)(uintptr_t)ast, &ret);
    kh_val(g_proto_registry, k) = p;
}

int ctr_use_vm = 0;

typedef struct localmap_entry {
    char*    name;
    uint32_t len;
    uint16_t slot;          // register slot when slot-promoted
    uint8_t  is_const;
    uint8_t  is_param;
    uint8_t  dict_only;     // declared via DECL_LOCAL in this proto's scope but not slot-promoted
} localmap_entry;

typedef struct bcgen {
    ctr_proto* p;
    // No escape nodes.
    int promotable;
    ctr_instr* code;     uint32_t n_code, cap_code;
    double*       nums;  uint32_t n_nums,  cap_nums;
    ctr_str_lit*  strs;  uint32_t n_strs,  cap_strs;
    ctr_tnode**   asts;  uint32_t n_asts,  cap_asts;
    ctr_proto**   protos;uint32_t n_protos,cap_protos;
    ctr_ic_slot*  ics;   uint32_t n_ics,   cap_ics;
    localmap_entry* locals; uint32_t n_locals, cap_locals;
    uint16_t reg_top;
    uint16_t reg_high;
    uint16_t reg_floor;
    struct bcgen* parent;
    ctr_upvar_desc* upvars;  uint32_t n_upvars, cap_upvars;
    int lexical;
} bcgen;

static void grow_code(bcgen* b)
{
    if (b->n_code < b->cap_code)
        return;
    uint32_t nc = b->cap_code ? b->cap_code * 2 : 64;
    ctr_instr* ni = (ctr_instr*)ctr_heap_allocate_tracked(sizeof(ctr_instr) * nc);
    if (b->cap_code) memcpy(ni, b->code, sizeof(ctr_instr) * b->n_code);
    b->code = ni;
    b->cap_code = nc;
}

#define GROW_GENERIC(field, type, init)                                \
static void grow_##field(bcgen* b) {                                   \
    if (b->n_##field < b->cap_##field) return;                         \
    uint32_t nc = b->cap_##field ? b->cap_##field * 2 : (init);        \
    type* nv = (type*)ctr_heap_allocate_tracked(sizeof(type) * nc);    \
    if (b->cap_##field) memcpy(nv, b->field, sizeof(type) * b->n_##field); \
    b->field = nv;                                                     \
    b->cap_##field = nc;                                               \
}

GROW_GENERIC(nums,    double,         8)
GROW_GENERIC(strs,    ctr_str_lit,    16)
GROW_GENERIC(asts,    ctr_tnode*,      8)
GROW_GENERIC(protos,  ctr_proto*,      4)
GROW_GENERIC(ics,     ctr_ic_slot,    16)
GROW_GENERIC(locals,  localmap_entry,  8)
GROW_GENERIC(upvars,  ctr_upvar_desc,  4)

static uint16_t reg_alloc(bcgen* b)
{
    uint16_t r = b->reg_top++;
    if (b->reg_top > b->reg_high) b->reg_high = b->reg_top;
    if (b->reg_top > 250) {
        fprintf(stderr, "bcgen: register pressure exceeded 250 in one frame\n");
        exit(1);
    }
    return r;
}

static uint16_t reg_alloc_n(bcgen* b, int n)
{
    uint16_t base = b->reg_top;
    b->reg_top += (uint16_t)n;
    if (b->reg_top > b->reg_high) b->reg_high = b->reg_top;
    if (b->reg_top > 250) {
        fprintf(stderr, "bcgen: register pressure exceeded 250 in one frame\n");
        exit(1);
    }
    return base;
}

static uint16_t reg_mark(bcgen* b) { return b->reg_top; }
static void     reg_restore(bcgen* b, uint16_t mark) { b->reg_top = mark; }

static uint32_t emit(bcgen* b, ctr_instr instr)
{
    grow_code(b);
    uint32_t pc = b->n_code;
    b->code[b->n_code++] = instr;
    if (getenv("CTR_BCGEN_TRACE")) {
        fprintf(stderr, "  emit pc=%u op=%u a=%u b=%u c=%u bx=%u (top=%u)\n",
            pc, (unsigned)CTR_OP(instr), (unsigned)CTR_A(instr), (unsigned)CTR_B(instr),
            (unsigned)CTR_C(instr), (unsigned)CTR_BX(instr), (unsigned)b->reg_top);
    }
    return pc;
}

static void patch_at(bcgen* b, uint32_t pc, ctr_instr instr)
{
    b->code[pc] = instr;
}

static uint16_t lit_num(bcgen* b, double v)
{
    for (uint32_t i = 0; i < b->n_nums; i++)
        if (b->nums[i] == v) return (uint16_t)i;
    grow_nums(b);
    b->nums[b->n_nums] = v;
    return (uint16_t)b->n_nums++;
}

static uint16_t lit_str(bcgen* b, const char* s, uint32_t len)
{
    for (uint32_t i = 0; i < b->n_strs; i++)
        if (b->strs[i].len == len && memcmp(b->strs[i].s, s, len) == 0)
            return (uint16_t)i;
    grow_strs(b);
    char* copy = (char*)ctr_heap_allocate_tracked(len + 1);
    memcpy(copy, s, len);
    copy[len] = 0;
    b->strs[b->n_strs].s = copy;
    b->strs[b->n_strs].len = len;
    return (uint16_t)b->n_strs++;
}

static uint16_t add_proto_ref(bcgen* b, ctr_proto* child)
{
    grow_protos(b);
    b->protos[b->n_protos] = child;
    return (uint16_t)b->n_protos++;
}

static uint16_t add_ast_ref(bcgen* b, ctr_tnode* node)
{
    grow_asts(b);
    b->asts[b->n_asts] = node;
    return (uint16_t)b->n_asts++;
}

static uint16_t alloc_ic(bcgen* b)
{
    grow_ics(b);
    memset(&b->ics[b->n_ics], 0, sizeof(ctr_ic_slot));
    return (uint16_t)b->n_ics++;
}

static uint16_t alloc_upvar(bcgen* b, uint16_t parent_slot)
{
    for (uint32_t i = 0; i < b->n_upvars; i++) {
        if (b->upvars[i].parent_depth == 0 && b->upvars[i].parent_slot == parent_slot)
            return (uint16_t)i;
    }
    grow_upvars(b);
    b->upvars[b->n_upvars].parent_depth = 0;
    b->upvars[b->n_upvars].parent_slot = parent_slot;
    return (uint16_t)b->n_upvars++;
}

static int find_local(bcgen* b, const char* name, uint32_t len)
{
    for (uint32_t i = 0; i < b->n_locals; i++)
        if (b->locals[i].len == len && memcmp(b->locals[i].name, name, len) == 0)
            return (int)i;
    return -1;
}

static int decl_local(bcgen* b, const char* name, uint32_t len, int is_const, int is_param)
{
    int existing = find_local(b, name, len);
    if (existing >= 0) {
        // re-declaration: keep the original entry
        return existing;
    }
    grow_locals(b);
    char* copy = (char*)ctr_heap_allocate_tracked(len + 1);
    memcpy(copy, name, len);
    copy[len] = 0;
    b->locals[b->n_locals].name     = copy;
    b->locals[b->n_locals].len      = len;
    b->locals[b->n_locals].slot     = 0; // unused
    b->locals[b->n_locals].is_const = (uint8_t)is_const;
    b->locals[b->n_locals].is_param = (uint8_t)is_param;
    b->locals[b->n_locals].dict_only = 0;
    return (int)b->n_locals++;
}

// Nodes that cannot be promoted:
// - RAW / EMBED: Compiletime stuff
// - NATIVEFN: Not safe to promote asm blocks
// - SYMBOL / IMMUTABLE / LISTCOMP: Escape to the AST interpreter
// - REFERENCE to a hole
// - REFERENCE 4/5 (frozen or pure)
// - destructuring
static int is_promotable_node(ctr_tnode* n);

static int is_promotable_list(ctr_tlistitem* it)
{
    for (; it; it = it->next) {
        if (it->node && !is_promotable_node(it->node))
            return 0;
    }
    return 1;
}

static int is_promotable_node(ctr_tnode* n)
{
    if (!n) return 1;
    switch (n->type) {
    case CTR_AST_NODE_RAW:
    case CTR_AST_NODE_EMBED:
    case CTR_AST_NODE_NATIVEFN:
    case CTR_AST_NODE_SYMBOL:
    case CTR_AST_NODE_IMMUTABLE:
    case CTR_AST_NODE_LISTCOMP:
        return 0;
    case CTR_AST_NODE_REFERENCE:
        if (n->modifier == 4 || n->modifier == 5) return 0;
        if (n->vlen == 1 && n->value && n->value[0] == '?') return 0;
        return 1;
    case CTR_AST_NODE_EXPRASSIGNMENT: {
        ctr_tnode* assignee = n->nodes ? n->nodes->node : NULL;
        if (!assignee || assignee->type != CTR_AST_NODE_REFERENCE) return 0;
        if (assignee->modifier == 4 || assignee->modifier == 5) return 0;
        return is_promotable_list(n->nodes);
    }
    case CTR_AST_NODE_CODEBLOCK:
        return 1;
    case CTR_AST_NODE_LTRSTRING:
    case CTR_AST_NODE_LTRNUM:
    case CTR_AST_NODE_LTRBOOLTRUE:
    case CTR_AST_NODE_LTRBOOLFALSE:
    case CTR_AST_NODE_LTRNIL:
        return 1;
    case CTR_AST_NODE_EXPRMESSAGE:
    case CTR_AST_NODE_UNAMESSAGE:
    case CTR_AST_NODE_BINMESSAGE:
    case CTR_AST_NODE_KWMESSAGE:
        return is_promotable_list(n->nodes);
    case CTR_AST_NODE_NESTED:
    case CTR_AST_NODE_INSTRLIST:
    case CTR_AST_NODE_PARAMLIST:
    case CTR_AST_NODE_RETURNFROMBLOCK:
        return is_promotable_list(n->nodes);
    case CTR_AST_NODE_PROGRAM:
        return is_promotable_list(n->nodes);
    case CTR_AST_NODE_ENDOFPROGRAM:
        return 1;
    default:
        return 0;
    }
}

static void compile_expr(bcgen* b, ctr_tnode* node, uint16_t dst);
static void compile_message(bcgen* b, ctr_tnode* msg_node, uint16_t dst);
static void compile_assignment(bcgen* b, ctr_tnode* node, uint16_t dst);
static ctr_proto* compile_block(ctr_tnode* block_node, bcgen* parent_b);
static void compile_instrlist(bcgen* b, ctr_tnode* instrlist);

static int ast_nested_references_name(ctr_tnode* n, const char* name, uint32_t len);
static int ast_list_nested_references_name(ctr_tlistitem* it, const char* name, uint32_t len)
{
    for (; it; it = it->next) {
        if (it->node && ast_nested_references_name(it->node, name, len))
            return 1;
    }
    return 0;
}

static int ast_references_name_including_blocks(ctr_tnode* n, const char* name, uint32_t len)
{
    if (!n) return 0;
    if (n->type == CTR_AST_NODE_REFERENCE) {
        return (n->modifier == 0 || n->modifier == 3)
            && (uint32_t)n->vlen == len
            && memcmp(n->value, name, len) == 0;
    }
    return ast_list_nested_references_name(n->nodes, name, len);
}

static int ast_nested_references_name(ctr_tnode* n, const char* name, uint32_t len)
{
    if (!n) return 0;
    if (n->type == CTR_AST_NODE_CODEBLOCK)
        return ast_references_name_including_blocks(n, name, len);
    return ast_list_nested_references_name(n->nodes, name, len);
}

// Try to promote all var and const locals to a register.
static void prescan_locals(bcgen* b, ctr_tlistitem* it)
{
    int slot_promote = b->promotable && b->p->is_program;
    ctr_tlistitem* root = it;
    for (; it; it = it->next) {
        ctr_tnode* stmt = it->node;
        if (!stmt) continue;
        if (stmt->type != CTR_AST_NODE_EXPRASSIGNMENT) continue;
        ctr_tnode* assignee = stmt->nodes ? stmt->nodes->node : NULL;
        if (!assignee || assignee->type != CTR_AST_NODE_REFERENCE) continue;
        if (assignee->modifier != 2 && assignee->modifier != 3) continue;
        if (find_local(b, assignee->value, (uint32_t)assignee->vlen) >= 0) continue;
        decl_local(b, assignee->value, (uint32_t)assignee->vlen, assignee->modifier == 3 ? 1 : 0, 0);
    }
    for (uint32_t i = 0; i < b->n_locals; i++) {
        if (slot_promote && !ast_list_nested_references_name(root, b->locals[i].name, b->locals[i].len)) {
            b->locals[i].slot = b->reg_floor;
            b->reg_floor = (uint16_t)(b->reg_floor + 1);
            b->reg_top = b->reg_floor;
            if (b->reg_top > b->reg_high) b->reg_high = b->reg_top;
        } else {
            b->locals[i].dict_only = 1;
        }
    }
}

static int ast_references_name(ctr_tnode* n, const char* name, uint32_t len);
static int ast_list_references_name(ctr_tlistitem* it, const char* name, uint32_t len)
{
    for (; it; it = it->next)
        if (it->node && ast_references_name(it->node, name, len))
            return 1;
    return 0;
}

static int ast_references_name(ctr_tnode* n, const char* name, uint32_t len)
{
    if (!n) return 0;
    if (n->type == CTR_AST_NODE_REFERENCE) {
        if ((n->modifier == 0 || n->modifier == 3)
            && (uint32_t)n->vlen == len
            && memcmp(n->value, name, len) == 0)
            return 1;
        return 0;
    }
    if (n->type == CTR_AST_NODE_CODEBLOCK) return 0; // different proto
    return ast_list_references_name(n->nodes, name, len);
}

static void emit_walk_ast(bcgen* b, ctr_tnode* node, uint16_t dst)
{
    uint16_t ix = add_ast_ref(b, node);
    emit(b, CTR_ENC_ABx(OP_WALK_AST, dst, ix));
}

static void compile_reference_load(bcgen* b, ctr_tnode* node, uint16_t dst)
{
    /* node->vlen == 1 && '?' is the unconstrained "hole" */
    if (node->vlen == 1 && node->value && node->value[0] == '?') {
        emit_walk_ast(b, node, dst);
        return;
    }

    if (b->promotable && node->modifier == 0) {
        int li = find_local(b, node->value, (uint32_t)node->vlen);
        if (li >= 0) {
            if (b->locals[li].dict_only) {
                // shadow declared via DECL_LOCAL in this block; must hit the dict so we see this block's binding, not parent's.
            } else {
                emit(b, CTR_ENC_AB(OP_MOV, dst, b->locals[li].slot));
                return;
            }
        } else {
            if (b->parent && b->parent->promotable && b->parent->p->is_program) {
                int pli = find_local(b->parent, node->value, (uint32_t)node->vlen);
                if (pli >= 0 && !b->parent->locals[pli].dict_only) {
                    uint16_t up_ix = alloc_upvar(b, b->parent->locals[pli].slot);
                    emit(b, CTR_ENC_AB(OP_LOAD_UP, dst, (uint8_t)up_ix));
                    return;
                }
            }
        }
    }

    uint16_t name_ix = lit_str(b, node->value, (uint32_t)node->vlen);
    int op;
    switch (node->modifier) {
    case 1: // my var
    case 3: // const var
        op = OP_LOAD_MY;
        break;
    case 0: // var
    default:
        op = OP_LOAD_LEX;
        break;
    }
    emit(b, CTR_ENC_ABx(op, dst, name_ix));
}

static void compile_codeblock(bcgen* b, ctr_tnode* node, uint16_t dst)
{
    ctr_proto* child = ctr_proto_for_ast(node);
    if (!child)
        child = compile_block(node, b);
    uint16_t ix = add_proto_ref(b, child);
    emit(b, CTR_ENC_ABx(OP_MAKE_BLOCK, dst, ix));
}

static void compile_instrlist(bcgen* b, ctr_tnode* instrlist)
{
    ctr_tlistitem* it = instrlist->nodes;
    if (!it) {
        emit(b, CTR_ENC_A(OP_RET_NIL, 0));
        return;
    }
    uint16_t last = 0;
    int produced_any = 0;
    while (it) {
        ctr_tnode* stmt = it->node;
        if (!stmt) { it = it->next; continue; }

        if (stmt->type == CTR_AST_NODE_RETURNFROMBLOCK) {
            uint16_t mark = reg_mark(b);
            uint16_t r = reg_alloc(b);
            ctr_tnode* inner = stmt->nodes ? stmt->nodes->node : NULL;
            if (inner)
                compile_expr(b, inner, r);
            else
                emit(b, CTR_ENC_A(OP_NIL, r));
            emit(b, CTR_ENC_A(OP_RET, r));
            reg_restore(b, mark);
            return;
        }

        uint16_t mark = reg_mark(b);
        uint16_t r = reg_alloc(b);
        compile_expr(b, stmt, r);
        last = r;
        produced_any = 1;
        // after each statement, check for flow disruption; if set, return immediately.
        // The walker runs the next statement only when CtrStdFlow == NULL (or a continue/break we don't interrupt for).
        emit(b, CTR_ENC_A(OP_CHECK_FLOW, r));
        if (it->next) {
            reg_restore(b, mark);
        }
        it = it->next;
    }
    if (b->lexical && produced_any) {
        emit(b, CTR_ENC_A(OP_RET, last));
    } else {
        emit(b, CTR_ENC_A(OP_RET_FALL, 0));
    }
}

/*
 * SEND lowering.
 *
 * EXPRMESSAGE shape:
 *   nodes->node         = receiver expression
 *   nodes->next         = first message (UNAMESSAGE / BINMESSAGE / KWMESSAGE)
 *   nodes->next->next   = second message (chained on the result)
 *   ...
 *
 * For each message:
 *   selector  = msg->value (built by parser, includes ":"s for keyword msgs)
 *   args      = msg->nodes (linked list of argument expressions; for KW messages, one node per keyword segment
 *
 * layout receiver in R, args in R+1..R+argc, emit OP_SEND r_dst R sym_ix + OP_SEND2 packed.
 */
static void compile_message(bcgen* b, ctr_tnode* msg_node, uint16_t dst)
{
    ctr_tlistitem* eitem = msg_node->nodes;
    ctr_tnode* receiverNode = eitem->node;

    uint16_t mark = reg_mark(b);
    uint16_t r_recv = reg_alloc(b);
    compile_expr(b, receiverNode, r_recv);

    ctr_tlistitem* li = eitem->next;
    uint16_t cur = r_recv;
    while (li) {
        ctr_tnode* m = li->node;

        int argc = 0;
        for (ctr_tlistitem* a = m->nodes; a; a = a->next)
            if (a->node) argc++;

        uint16_t base = reg_alloc_n(b, argc + 1);
        emit(b, CTR_ENC_AB(OP_MOV, base, cur));
        int i = 0;
        for (ctr_tlistitem* a = m->nodes; a; a = a->next) {
            if (!a->node) continue;
            compile_expr(b, a->node, (uint16_t)(base + 1 + i));
            i++;
        }

        uint16_t sel_ix = lit_str(b, m->value, (uint32_t)m->vlen);
        uint16_t ic_ix  = alloc_ic(b);
        uint16_t out    = li->next ? reg_alloc(b) : dst;

        emit(b, CTR_ENC_SEND_W1(out, base, (uint8_t)argc));
        emit(b, CTR_ENC_SEND_W2(sel_ix, ic_ix));

        cur = out;
        li = li->next;
    }

    if (cur != dst)
        emit(b, CTR_ENC_AB(OP_MOV, dst, cur));

    reg_restore(b, mark);
}

static void compile_assignment(bcgen* b, ctr_tnode* node, uint16_t dst)
{
    ctr_tlistitem* items = node->nodes;
    ctr_tnode* assignee = items->node;
    ctr_tnode* value    = items->next->node;

    // destructuring, just let the ast interpreter handle it.
    if (assignee->type != CTR_AST_NODE_REFERENCE) {
        emit_walk_ast(b, node, dst);
        return;
    }

    uint16_t mark = reg_mark(b);
    uint16_t v = reg_alloc(b);
    compile_expr(b, value, v);

    if (b->promotable && (assignee->modifier == 0 || assignee->modifier == 2 || assignee->modifier == 3)) {
        int li = find_local(b, assignee->value, (uint32_t)assignee->vlen);
        if (li >= 0 && b->locals[li].dict_only) li = -1;
        if (li >= 0) {
            uint16_t slot = b->locals[li].slot;
            uint16_t name_ix = lit_str(b, assignee->value, (uint32_t)assignee->vlen);
            int dict_op;
            switch (assignee->modifier) {
            case 2: dict_op = OP_DECL_LOCAL; break;
            case 3: dict_op = OP_DECL_CONST; break;
            default: dict_op = OP_STORE_LEX; break;
            }
            emit(b, CTR_ENC_AB(OP_MOV, slot, v));
            emit(b, CTR_ENC_ABx(dict_op, v, name_ix));
            if (dst != v)
                emit(b, CTR_ENC_AB(OP_MOV, dst, v));
            reg_restore(b, mark);
            return;
        }
        if (assignee->modifier == 0 && find_local(b, assignee->value, (uint32_t)assignee->vlen) < 0 && b->parent && b->parent->promotable && b->parent->p->is_program) {
            int pli = find_local(b->parent, assignee->value, (uint32_t)assignee->vlen);
            if (pli >= 0 && !b->parent->locals[pli].dict_only) {
                uint16_t up_ix = alloc_upvar(b, b->parent->locals[pli].slot);
                uint16_t name_ix = lit_str(b, assignee->value, (uint32_t)assignee->vlen);
                emit(b, CTR_ENC_AB(OP_STORE_UP, (uint8_t)up_ix, v));
                emit(b, CTR_ENC_ABx(OP_STORE_LEX, v, name_ix));
                if (dst != v)
                    emit(b, CTR_ENC_AB(OP_MOV, dst, v));
                reg_restore(b, mark);
                return;
            }
        }
    }

    uint16_t name_ix = lit_str(b, assignee->value, (uint32_t)assignee->vlen);

    int op;
    switch (assignee->modifier) {
    case 1: op = OP_STORE_MY;    break;
    case 2: op = OP_DECL_LOCAL; // no slot
        break;
    case 3: op = OP_DECL_CONST; // no slot
        break;
    case 4:
    case 5:
        emit_walk_ast(b, node, dst);
        reg_restore(b, mark);
        return;
    case 0:
    default:
        op = OP_STORE_LEX;
        break;
    }
    emit(b, CTR_ENC_ABx(op, v, name_ix));

    if (dst != v)
        emit(b, CTR_ENC_AB(OP_MOV, dst, v));
    reg_restore(b, mark);
}

static void compile_expr(bcgen* b, ctr_tnode* node, uint16_t dst)
{
    if (!node) {
        emit(b, CTR_ENC_A(OP_NIL, dst));
        return;
    }

    switch (node->type) {
    case CTR_AST_NODE_LTRNIL:
        emit(b, CTR_ENC_A(OP_NIL, dst));
        return;
    case CTR_AST_NODE_LTRBOOLTRUE:
        emit(b, CTR_ENC_A(OP_TRUE, dst));
        return;
    case CTR_AST_NODE_LTRBOOLFALSE:
        emit(b, CTR_ENC_A(OP_FALSE, dst));
        return;
    case CTR_AST_NODE_LTRSTRING: {
        uint16_t ix = lit_str(b, node->value, (uint32_t)node->vlen);
        emit(b, CTR_ENC_ABx(OP_KSTR, dst, ix));
        return;
    }
    case CTR_AST_NODE_LTRNUM: {
        char buf[64];
        size_t n = (size_t)node->vlen < sizeof(buf) - 1 ? (size_t)node->vlen : sizeof(buf) - 1;
        memcpy(buf, node->value, n);
        buf[n] = 0;
        double d = strtod(buf, NULL);
        uint16_t ix = lit_num(b, d);
        emit(b, CTR_ENC_ABx(OP_KNUM, dst, ix));
        return;
    }
    case CTR_AST_NODE_REFERENCE:
        compile_reference_load(b, node, dst);
        return;
    case CTR_AST_NODE_EXPRMESSAGE:
        compile_message(b, node, dst);
        return;
    case CTR_AST_NODE_EXPRASSIGNMENT:
        compile_assignment(b, node, dst);
        return;
    case CTR_AST_NODE_CODEBLOCK:
        compile_codeblock(b, node, dst);
        return;
    case CTR_AST_NODE_NESTED:
        compile_expr(b, node->nodes->node, dst);
        return;
    case CTR_AST_NODE_RETURNFROMBLOCK: {
        uint16_t r = reg_alloc(b);
        compile_expr(b, node->nodes->node, r);
        emit(b, CTR_ENC_A(OP_RET, r));
        return;
    }

    case CTR_AST_NODE_IMMUTABLE:
    case CTR_AST_NODE_LISTCOMP:
    case CTR_AST_NODE_RAW:
    case CTR_AST_NODE_EMBED:
    case CTR_AST_NODE_SYMBOL:
    case CTR_AST_NODE_NATIVEFN:
    case CTR_AST_NODE_INSTRLIST:
        emit_walk_ast(b, node, dst);
        return;

    default:
        fprintf(stderr, "bcgen: unhandled AST node type %d\n", node->type);
        emit_walk_ast(b, node, dst);
        return;
    }
}

static ctr_proto* finalize(bcgen* b)
{
    ctr_proto* p = b->p;

#define MOVE_ARR(src_field, dst_field, type)                       \
    do {                                                           \
        if (b->n_##src_field) {                                    \
            p->dst_field = (type*)ctr_heap_allocate_tracked(       \
                sizeof(type) * b->n_##src_field);                  \
            memcpy(p->dst_field, b->src_field,                     \
                sizeof(type) * b->n_##src_field);                  \
        } else {                                                   \
            p->dst_field = NULL;                                   \
        }                                                          \
        p->n_##dst_field = b->n_##src_field;                       \
    } while (0)

    MOVE_ARR(code,    code,   ctr_instr);
    MOVE_ARR(nums,    nums,   double);
    MOVE_ARR(strs,    strs,   ctr_str_lit);
    MOVE_ARR(asts,    asts,   ctr_tnode*);
    MOVE_ARR(protos,  protos, ctr_proto*);
    MOVE_ARR(ics,     ics,    ctr_ic_slot);

#undef MOVE_ARR

    if (b->n_upvars) {
        p->upvar_desc = (ctr_upvar_desc*)ctr_heap_allocate_tracked(sizeof(ctr_upvar_desc) * b->n_upvars);
        memcpy(p->upvar_desc, b->upvars, sizeof(ctr_upvar_desc) * b->n_upvars);
    } else {
        p->upvar_desc = NULL;
    }
    p->n_upvars = (uint16_t)b->n_upvars;

    p->n_args   = (uint16_t)0; // set by caller
    p->n_locals = (uint16_t)b->n_locals;
    p->n_upvars = 0;
    p->n_regs   = b->reg_high;
    p->local_base = (uint16_t)(1 + p->n_args);
    p->reg_base   = (uint16_t)(p->local_base + p->n_locals);

    return p;
}

static uint32_t g_proto_id_counter = 1;

static ctr_proto* alloc_proto(ctr_tnode* ast)
{
    ctr_proto* p = (ctr_proto*)ctr_heap_allocate_tracked(sizeof(ctr_proto));
    memset(p, 0, sizeof(*p));
    p->id = g_proto_id_counter++;
    p->ast = ast;
    return p;
}

ctr_proto* ctr_bcgen_program(struct ctr_tnode* program)
{
    if (!program) return NULL;
    ctr_proto* existing = ctr_proto_for_ast(program);
    if (existing) return existing;

    ctr_proto* p = alloc_proto(program);
    p->is_program = 1;
    ctr_proto_register(program, p);

    bcgen b = {0};
    b.p = p;
    b.promotable = is_promotable_node(program);
    // r0 for `me'.
    b.reg_top = 1;
    b.reg_high = 1;
    b.reg_floor = 1;

    prescan_locals(&b, program->nodes);

    ctr_tlistitem* it = program->nodes;
    uint16_t last_reg = 0;
    int produced = 0;
    while (it) {
        ctr_tnode* stmt = it->node;
        if (!stmt) { it = it->next; continue; }
        if (stmt->type == CTR_AST_NODE_ENDOFPROGRAM) {
            uint16_t mark = reg_mark(&b);
            uint16_t r = reg_alloc(&b);
            emit_walk_ast(&b, stmt, r);
            reg_restore(&b, mark);
            break;
        }
        uint16_t mark = reg_mark(&b);
        uint16_t r = reg_alloc(&b);
        compile_expr(&b, stmt, r);
        last_reg = r;
        produced = 1;
        emit(&b, CTR_ENC_A(OP_CHECK_FLOW, r));
        if (it->next) reg_restore(&b, mark);
        it = it->next;
    }
    if (produced)
        emit(&b, CTR_ENC_A(OP_RET, last_reg));
    else
        emit(&b, CTR_ENC_A(OP_RET_NIL, 0));

    finalize(&b);
    p->promotable = b.promotable;
    p->n_args = 0;
    p->local_base = 1;
    p->reg_base = (uint16_t)(p->local_base + p->n_locals);
    return p;
}

ctr_proto* ctr_bcgen_block(struct ctr_tnode* codeblock, ctr_proto* parent)
{
    (void)parent;
    return compile_block(codeblock, NULL);
}

static ctr_proto* compile_block(ctr_tnode* block_node, bcgen* parent_b)
{
    ctr_proto* existing = ctr_proto_for_ast(block_node);
    if (existing) return existing;

    ctr_proto* p = alloc_proto(block_node);
    p->parent = parent_b ? parent_b->p : NULL;
    ctr_proto_register(block_node, p);

    // CODEBLOCK.nodes->node = PARAMLIST (REFERENCEs);
    //           ->next->node = INSTRLIST (statements).
    ctr_tnode* paramlist = NULL;
    ctr_tnode* instrlist = NULL;
    if (block_node->nodes) {
        paramlist = block_node->nodes->node;
        if (block_node->nodes->next)
            instrlist = block_node->nodes->next->node;
    }

    bcgen b = {0};
    b.p = p;
    b.parent = parent_b;
    b.promotable = instrlist ? is_promotable_list(instrlist->nodes) : 1;
    b.lexical = block_node->lexical;

    int n_args = 0;
    if (paramlist) {
        for (ctr_tlistitem* it = paramlist->nodes; it; it = it->next) {
            if (it->node) n_args++;
        }
    }
    p->n_args = (uint16_t)n_args;

    b.reg_top = (uint16_t)(1 + n_args);
    b.reg_high = b.reg_top;
    b.reg_floor = b.reg_top;

    if (instrlist)
        prescan_locals(&b, instrlist->nodes);

    if (instrlist) {
        compile_instrlist(&b, instrlist);
    } else {
        emit(&b, CTR_ENC_A(OP_RET_NIL, 0));
    }

    finalize(&b);
    p->promotable = b.promotable;
    p->n_args = (uint16_t)n_args;
    p->n_locals = (uint16_t)b.n_locals;
    p->local_base = (uint16_t)(1 + p->n_args);
    p->reg_base   = (uint16_t)(p->local_base + p->n_locals);
    p->n_regs     = b.reg_high;
    return p;
}

static const char* op_name(int op)
{
    switch (op) {
#define X(o) case o: return #o;
    X(OP_NIL) X(OP_TRUE) X(OP_FALSE) X(OP_KSELF)
    X(OP_KNUM) X(OP_KSTR) X(OP_MOV)
    X(OP_LOAD_MY) X(OP_STORE_MY) X(OP_LOAD_LEX) X(OP_STORE_LEX)
    X(OP_DECL_LOCAL) X(OP_DECL_CONST) X(OP_LOAD_UP) X(OP_STORE_UP)
    X(OP_MAKE_BLOCK) X(OP_BUILD_TUPLE) X(OP_BUILD_LISTCOMP)
    X(OP_SEND)
    X(OP_JMP) X(OP_JMP_IF) X(OP_JMP_IF_NOT) X(OP_RET) X(OP_RET_NIL) X(OP_RET_FALL)
    X(OP_WALK_AST) X(OP_RAISE) X(OP_CHECK_FLOW)
#undef X
    default: return "OP_?";
    }
}

void ctr_proto_dump(ctr_proto* p, int indent)
{
    if (!p) { printf("%*s(null proto)\n", indent, ""); return; }
    printf("%*sproto #%u (%s) args=%u locals=%u regs=%u code=%u\n",
        indent, "", p->id,
        p->is_program ? "program" : "block",
        p->n_args, p->n_locals, p->n_regs, p->n_code);
    for (uint32_t pc = 0; pc < p->n_code; pc++) {
        ctr_instr ins = p->code[pc];
        int op = (int)CTR_OP(ins);
        printf("%*s  %04u: %-14s a=%u b=%u c=%u bx=%u ax=%d\n",
            indent, "", pc, op_name(op),
            CTR_A(ins), CTR_B(ins), CTR_C(ins),
            CTR_BX(ins), CTR_AX(ins));
    }
    for (uint32_t i = 0; i < p->n_protos; i++) {
        printf("%*s  -- child %u --\n", indent, "", i);
        ctr_proto_dump(p->protos[i], indent + 4);
    }
}
