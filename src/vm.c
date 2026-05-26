#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "citron.h"
#include "bytecode.h"
#include "khash.h"

typedef struct ctr_upvar_binding {
    ctr_object** slots;   /* n entries; each is &parent_regs[parent_slot]  */
    uint16_t     n;
} ctr_upvar_binding;

KHASH_MAP_INIT_INT64(blockup, ctr_upvar_binding*)
static khash_t(blockup)* g_blockup = NULL;

__thread ctr_object** ctr_vm_program_regs = NULL;

static void blockup_set(ctr_object* blk, ctr_upvar_binding* ub)
{
    if (!g_blockup) g_blockup = kh_init(blockup);
    int ret;
    khiter_t k = kh_put(blockup, g_blockup, (uint64_t)(uintptr_t)blk, &ret);
    kh_val(g_blockup, k) = ub;
}

static ctr_upvar_binding* blockup_get(ctr_object* blk)
{
    if (!g_blockup || !blk) return NULL;
    khiter_t k = kh_get(blockup, g_blockup, (uint64_t)(uintptr_t)blk);
    if (k == kh_end(g_blockup)) return NULL;
    return kh_val(g_blockup, k);
}

static ctr_object* mk_name_obj(ctr_str_lit s)
{
    ctr_object* o = ctr_build_string(s.s, (long)s.len);
    o->info.sticky = 1;
    return o;
}

#define CTR_VM_ARG_INLINE 8

static ctr_argument* build_arglist_inline(ctr_argument* buf, ctr_object** regs, uint16_t base, int argc)
{
    if (argc == 0) {
        buf[0].object = CtrStdNil;
        buf[0].next = NULL;
        return buf;
    }
    for (int i = 0; i < argc; i++) {
        buf[i].object = regs[base + 1 + i];
        buf[i].next = &buf[i + 1];
    }
    buf[argc].object = NULL;
    buf[argc].next = NULL;
    return buf;
}

// If we can't fit the args in CTR_VM_ARG_INLINE, allocate on the heap.
static ctr_argument* build_arglist_heap(ctr_object** regs, uint16_t base, int argc)
{
    ctr_argument* head = (ctr_argument*)ctr_heap_allocate(sizeof(ctr_argument));
    head->object = CtrStdNil;
    head->next = NULL;
    if (argc == 0) return head;
    ctr_argument* cur = head;
    for (int i = 0; i < argc; i++) {
        cur->object = regs[base + 1 + i];
        cur->next = (ctr_argument*)ctr_heap_allocate(sizeof(ctr_argument));
        cur = cur->next;
        cur->object = NULL;
        cur->next = NULL;
    }
    return head;
}

static void free_arglist_heap(ctr_argument* a)
{
    while (a) {
        ctr_argument* n = a->next;
        ctr_heap_free(a);
        a = n;
    }
}

static int is_truthy(ctr_object* o)
{
    if (!o) return 0;
    if (o->info.type == CTR_OBJECT_TYPE_OTBOOL) return o->value.bvalue != 0;
    if (o->info.type == CTR_OBJECT_TYPE_OTNIL) return 0;

    ctr_object* b = ctr_internal_cast2bool(o);
    return b && b->info.type == CTR_OBJECT_TYPE_OTBOOL && b->value.bvalue;
}

static ctr_object* run_proto(ctr_proto* p, ctr_object* my, ctr_object* myself);

ctr_object* ctr_vm_run_program(ctr_proto* p)
{
    if (!p) return CtrStdNil;
    ctr_object** saved = ctr_vm_program_regs;
    ctr_object* result;
    result = run_proto(p, NULL, NULL);
    ctr_vm_program_regs = saved;
    return result;
}

ctr_object* ctr_vm_run_block(ctr_object* block_obj, ctr_argument* args, ctr_object* my)
{
    (void)args;
    if (!block_obj) return CtrStdNil;
    ctr_proto* p = ctr_proto_for_ast(block_obj->value.block);
    ctr_object* result;
    if (p && p->promotable) {
        result = run_proto(p, my, block_obj);
    } else {
        ctr_tnode* body = block_obj->value.block->nodes->next->node;
        result = ctr_cwlk_run(body);
    }
    if (result == NULL)
        return my ? my : block_obj;
    return result;
}

static ctr_object* run_proto(ctr_proto* p, ctr_object* my, ctr_object* myself)
{
    uint32_t nregs = p->n_regs ? p->n_regs : 1;
    ctr_object** regs = (ctr_object**)ctr_heap_allocate(sizeof(ctr_object*) * nregs);
    for (uint32_t i = 0; i < nregs; i++) regs[i] = CtrStdNil;
    // If we have a self/me; r[0] holds it.
    if (my) regs[0] = my;
    else if (myself) regs[0] = myself;

    ctr_upvar_binding* ub = myself ? blockup_get(myself) : NULL;

    if (p->is_program)
        ctr_vm_program_regs = regs;

    ctr_object* result = CtrStdNil;
    ctr_instr* code = p->code;
    uint32_t pc = 0;
    while (pc < p->n_code) {
        ctr_instr ins = code[pc++];
        int op = (int)CTR_OP(ins);
        switch (op) {

        case OP_NIL:
            regs[CTR_A(ins)] = CtrStdNil;
            break;
        case OP_TRUE:
            regs[CTR_A(ins)] = ctr_build_bool(1);
            break;
        case OP_FALSE:
            regs[CTR_A(ins)] = ctr_build_bool(0);
            break;
        case OP_KSELF:
            regs[CTR_A(ins)] = ctr_find(ctr_build_string_from_cstring("me"));
            break;
        case OP_KNUM:
            regs[CTR_A(ins)] = ctr_build_number_from_float(p->nums[CTR_BX(ins)]);
            break;
        case OP_KSTR: {
            ctr_str_lit s = p->strs[CTR_BX(ins)];
            regs[CTR_A(ins)] = ctr_build_string(s.s, (long)s.len);
            break;
        }
        case OP_MOV:
            regs[CTR_A(ins)] = regs[CTR_B(ins)];
            break;

        case OP_LOAD_MY: {
            ctr_object* name = mk_name_obj(p->strs[CTR_BX(ins)]);
            regs[CTR_A(ins)] = ctr_find_in_my(name);
            break;
        }
        case OP_STORE_MY: {
            ctr_object* name = mk_name_obj(p->strs[CTR_BX(ins)]);
            ctr_assign_value_to_my(name, regs[CTR_A(ins)]);
            break;
        }
        case OP_LOAD_LEX: {
            ctr_object* name = mk_name_obj(p->strs[CTR_BX(ins)]);
            regs[CTR_A(ins)] = ctr_find(name);
            break;
        }
        case OP_STORE_LEX: {
            ctr_object* name = mk_name_obj(p->strs[CTR_BX(ins)]);
            ctr_assign_value(name, regs[CTR_A(ins)]);
            break;
        }
        case OP_DECL_LOCAL: {
            ctr_object* name = mk_name_obj(p->strs[CTR_BX(ins)]);
            ctr_assign_value_to_local(name, regs[CTR_A(ins)]);
            break;
        }
        case OP_DECL_CONST: {
            ctr_object* name = mk_name_obj(p->strs[CTR_BX(ins)]);
            ctr_const_assign_value(name, regs[CTR_A(ins)],
                                   ctr_contexts[ctr_context_id]);
            break;
        }

        case OP_MAKE_BLOCK: {
            ctr_proto* child = p->protos[CTR_BX(ins)];
            ctr_object* blk = ctr_build_block(child->ast);
            regs[CTR_A(ins)] = blk;
            if (child->n_upvars > 0) {
                ctr_upvar_binding* ub = (ctr_upvar_binding*) ctr_heap_allocate(sizeof(*ub));
                ub->n = child->n_upvars;
                ub->slots = (ctr_object**)ctr_heap_allocate(sizeof(ctr_object*) * ub->n);
                for (uint16_t i = 0; i < ub->n; i++) {
                    uint16_t pslot = child->upvar_desc[i].parent_slot;
                    ub->slots[i] = (ctr_object*)(uintptr_t)pslot; /* see below */
                }
                blockup_set(blk, ub);
            }
            break;
        }
        case OP_LOAD_UP:
            if (ub) {
                extern __thread ctr_object** ctr_vm_program_regs;
                if (ctr_vm_program_regs) {
                    uint16_t pslot = (uint16_t)(uintptr_t)ub->slots[CTR_B(ins)];
                    regs[CTR_A(ins)] = ctr_vm_program_regs[pslot];
                } else {
                    regs[CTR_A(ins)] = CtrStdNil;
                }
            } else {
                regs[CTR_A(ins)] = CtrStdNil;
            }
            break;
        case OP_STORE_UP:
            if (ub) {
                extern __thread ctr_object** ctr_vm_program_regs;
                if (ctr_vm_program_regs) {
                    uint16_t pslot = (uint16_t)(uintptr_t)ub->slots[CTR_B(ins)];
                    ctr_vm_program_regs[pslot] = regs[CTR_A(ins)];
                }
            }
            break;

        case OP_SEND: {
            uint8_t  dst     = CTR_A(ins);
            uint8_t  argbase = CTR_B(ins);
            uint8_t  argc    = CTR_C(ins);
            if (pc >= p->n_code) {
                fprintf(stderr, "vm: truncated SEND at pc=%u\n", pc - 1);
                exit(1);
            }
            ctr_instr w2 = code[pc++];
            uint16_t sel_ix = CTR_SEND_SEL(w2);
            uint16_t ic_ix  = CTR_SEND_IC(w2);

            ctr_str_lit sel = p->strs[sel_ix];
            ctr_object* recv = regs[argbase];
            ctr_ic_slot* ic = &p->ics[ic_ix];

            ctr_object* res = NULL;

            ctr_argument inline_buf[CTR_VM_ARG_INLINE + 1];
            ctr_argument* args;
            int use_heap = (argc > CTR_VM_ARG_INLINE);
            args = use_heap
                ? build_arglist_heap(regs, argbase, argc)
                : build_arglist_inline(inline_buf, regs, argbase, argc);

            if (ic->state == CTR_IC_MONO && recv && recv->info.type == ic->shape_or_kind && recv->info.type != CTR_OBJECT_TYPE_OTOBJECT && !recv->info.asyncMode) {
                ctr_object* method = (ctr_object*)ic->resolved;
                if (method->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
                    res = method->value.fvalue(recv, args);
                } else {
                    res = ctr_block_run(method, args, recv);
                }
                ic->hits++;
            } else {
                if (ic->state == CTR_IC_UNINIT && recv && recv->info.type != CTR_OBJECT_TYPE_OTOBJECT && !recv->info.asyncMode) {
                    ctr_object* method = ctr_get_responder(recv, sel.s, (long)sel.len);
                    if (method && (method->info.type == CTR_OBJECT_TYPE_OTNATFUNC || method->info.type == CTR_OBJECT_TYPE_OTBLOCK)) {
                        ic->resolved = method;
                        ic->shape_or_kind = recv->info.type;
                        ic->state = CTR_IC_MONO;
                        ic->arity = (uint8_t)argc;
                    }
                }
                res = ctr_send_message(recv, sel.s, (long)sel.len, args);
            }
            if (use_heap) free_arglist_heap(args);

            regs[dst] = res ? res : CtrStdNil;
            break;
        }

        case OP_JMP: {
            int32_t off = CTR_AX(ins);
            pc = (uint32_t)((int32_t)pc + off);
            break;
        }
        case OP_JMP_IF: {
            int32_t off = CTR_AX(ins);
            if (is_truthy(regs[CTR_A(ins)]))
                pc = (uint32_t)((int32_t)pc + off);
            break;
        }
        case OP_JMP_IF_NOT: {
            int32_t off = CTR_AX(ins);
            if (!is_truthy(regs[CTR_A(ins)]))
                pc = (uint32_t)((int32_t)pc + off);
            break;
        }

        case OP_RET:
            result = regs[CTR_A(ins)] ? regs[CTR_A(ins)] : CtrStdNil;
            goto done;
        case OP_RET_NIL:
            result = CtrStdNil;
            goto done;
        case OP_RET_FALL:
            // fell through without ^; mirror the walker's "result = NULL" path so ctr_internal_run_block converts to my/myself.
            result = NULL;
            goto done;

        case OP_WALK_AST: {
            ctr_tnode* node = p->asts[CTR_BX(ins)];
            char wasReturn = 0;
            ctr_object* res = ctr_cwlk_expr(node, &wasReturn);
            regs[CTR_A(ins)] = res ? res : CtrStdNil;
            if (wasReturn) {
                result = regs[CTR_A(ins)];
                goto done;
            }
            break;
        }
        case OP_RAISE:
            CtrStdFlow = regs[CTR_A(ins)];
            result = CtrStdNil;
            goto done;
        case OP_CHECK_FLOW:
            if (ctr_internal_next_return) {
                ctr_internal_next_return = 0;
                result = regs[CTR_A(ins)] ? regs[CTR_A(ins)] : CtrStdNil;
                goto done;
            }
            if (CtrStdFlow != NULL) {
                result = CtrStdNil;
                goto done;
            }
            break;

        default:
            fprintf(stderr, "vm: unhandled opcode %d at pc=%u\n", op, pc - 1);
            exit(1);
        }
    }
done:
    ctr_heap_free(regs);
    return result;
}

ctr_object* ctr_vm_run_proto(ctr_proto* p, ctr_object* my, ctr_object* myself)
{
    return run_proto(p, my, myself);
}
