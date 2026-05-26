#ifndef CTR_BYTECODE_H
#define CTR_BYTECODE_H

#include <stddef.h>
#include <stdint.h>

struct ctr_object;
struct ctr_tnode;
struct ctr_argument;

typedef struct ctr_str_lit {
    char*    s;
    uint32_t len;
} ctr_str_lit;

typedef uint32_t ctr_instr;

/* ------------------------------------------------------------------------ */
/* opcodes                                                                  */
/* ------------------------------------------------------------------------ */

enum ctr_opcode {
    /* loads / moves */
    OP_NIL = 0,        /* A         : r[A] = nil                              */
    OP_TRUE,           /* A         : r[A] = true                             */
    OP_FALSE,          /* A         : r[A] = false                            */
    OP_KSELF,          /* A         : r[A] = me (slot 0)                      */
    OP_KNUM,           /* A Bx      : r[A] = number_from_double(nums[Bx])     */
    OP_KSTR,           /* A Bx      : r[A] = string_from(strs[Bx])            */
    OP_MOV,            /* A B       : r[A] = r[B]                             */

    /* scopes (Bx indexes into strs[])                                       */
    OP_LOAD_MY,        /* A Bx      : r[A] = ctr_find_in_my(strs[Bx])         */
    OP_STORE_MY,       /* A Bx      : assign_to_my(strs[Bx], r[A])            */
    OP_LOAD_LEX,       /* A Bx      : r[A] = ctr_find(strs[Bx])               */
    OP_STORE_LEX,      /* A Bx      : assign_value(strs[Bx], r[A])            */
    OP_DECL_LOCAL,     /* A Bx      : declare local strs[Bx] = r[A]           */
    OP_DECL_CONST,     /* A Bx      : declare const strs[Bx] = r[A]           */
    OP_LOAD_UP,        /* A B       : r[A] = upvars[B]                        */
    OP_STORE_UP,       /* A B       : upvars[B] = r[A]                        */

    /* aggregates / blocks                                                   */
    OP_MAKE_BLOCK,     /* A Bx      : r[A] = block_for(protos[Bx])            */
    OP_BUILD_TUPLE,    /* A B C     : r[A] = tuple of r[B..B+C-1]             */
    OP_BUILD_LISTCOMP, /* A Bx      : r[A] = ctr_build_listcomp(asts[Bx])     */

    /* call (2-word encoding)                                                */
    /*   word 1: op(8) | dst(8) | argbase(8) | argc(8)                       */
    /*   word 2: sel_ix(16) | ic_ix(16)                                      */
    OP_SEND,

    /* control                                                                */
    OP_JMP,            /* Ax        : pc += Ax (signed)                       */
    OP_JMP_IF,         /* A Ax      : if truthy(r[A]) pc += Ax                */
    OP_JMP_IF_NOT,     /* A Ax      : if falsy(r[A])  pc += Ax                */
    OP_RET,            /* A         : return r[A]   (explicit ^ return)       */
    OP_RET_NIL,        /*           : return nil    (explicit return)         */
    OP_RET_FALL,       /* A         : block fell through (implicit return)    */

    /* escape / exception                                                     */
    OP_WALK_AST,       /* A Bx      : r[A] = walker(asts[Bx]); jit must bail  */
    OP_RAISE,          /* A         : CtrStdFlow = r[A]                       */
    OP_CHECK_FLOW,     /* Ax        : if CtrStdFlow then bubble (return)      */

    /* trace-only (forward-declared; not emitted by baseline) ---------------*/
    OP_GRD_KIND,
    OP_GRD_SHAPE,
    OP_GRD_METHOD,
    OP_GRD_SAME,
    OP_GRD_NUM,
    OP_GRD_NO_FLOW,
    OP_EXIT,
    OP_TRIP,
    OP_NADD, OP_NSUB, OP_NMUL, OP_NDIV, OP_NMOD,
    OP_NLT,  OP_NLE,  OP_NEQ,

    OP__MAX
};

/* instruction encoding                                                     */
/*   layout: op:8 | A:8 | B:8 | C:8                                         */
/*   variants: A + Bx:16 ; Ax:24 (signed)                                   */

#define CTR_OP(i)   ((uint32_t)((i) >> 24) & 0xff)
#define CTR_A(i)    ((uint32_t)((i) >> 16) & 0xff)
#define CTR_B(i)    ((uint32_t)((i) >> 8)  & 0xff)
#define CTR_C(i)    ((uint32_t)( i)        & 0xff)
#define CTR_BX(i)   ((uint32_t)( i)        & 0xffff)
#define CTR_AX(i)   (((int32_t)((i) << 8)) >> 8)

#define CTR_ENC_A(op, a)            ((((uint32_t)(op)) << 24) | (((uint32_t)(a)) << 16))
#define CTR_ENC_AB(op, a, b)        (CTR_ENC_A(op, a) | (((uint32_t)(b)) << 8))
#define CTR_ENC_ABC(op, a, b, c)    (CTR_ENC_AB(op, a, b) | ((uint32_t)(c) & 0xff))
#define CTR_ENC_ABx(op, a, bx)      (CTR_ENC_A(op, a) | ((uint32_t)(bx) & 0xffff))
#define CTR_ENC_Ax(op, ax)          ((((uint32_t)(op)) << 24) | ((uint32_t)(ax) & 0x00ffffff))

/* SEND word 1: op(8) | dst(8) | argbase(8) | argc(8)                       */
/* SEND word 2:        sel_ix(16)        |        ic_ix(16)                 */
#define CTR_SEND_DST(w)      (((uint32_t)(w) >> 16) & 0xff)
#define CTR_SEND_ARGBASE(w)  (((uint32_t)(w) >>  8) & 0xff)
#define CTR_SEND_ARGC(w)     ( (uint32_t)(w)        & 0xff)
#define CTR_SEND_SEL(w2)     (((uint32_t)(w2) >> 16) & 0xffff)
#define CTR_SEND_IC(w2)      ( (uint32_t)(w2)        & 0xffff)
#define CTR_ENC_SEND_W1(dst, argbase, argc) \
    ((((uint32_t)OP_SEND) << 24) \
     | (((uint32_t)(dst) & 0xff) << 16) \
     | (((uint32_t)(argbase) & 0xff) << 8) \
     | ((uint32_t)(argc) & 0xff))
#define CTR_ENC_SEND_W2(sel, ic) \
    ((((uint32_t)(sel) & 0xffff) << 16) | ((uint32_t)(ic) & 0xffff))

#define CTR_IC_UNINIT 0
#define CTR_IC_MONO   1
#define CTR_IC_POLY   2
#define CTR_IC_MEGA   3

typedef struct ctr_ic_slot {
    void*    resolved;       /* method ctr_object* or native fn ptr */
    uint32_t shape_or_kind;  /* OBJECT: shape id ; else type tag    */
    uint32_t version;
    uint8_t  state;
    uint8_t  arity;
    uint16_t hits;
} ctr_ic_slot;

typedef struct ctr_upvar_desc {
    uint16_t parent_depth;   /* 0 = enclosing proto, 1 = its enclosing, ... */
    uint16_t parent_slot;    /* register slot in that proto's frame         */
} ctr_upvar_desc;

typedef struct ctr_exit_record {
    struct ctr_proto* baseline_proto;
    uint32_t          baseline_pc;
    uint32_t          n_reg_map;
    int16_t*          reg_map;        /* trace_reg -> baseline_slot         */
} ctr_exit_record;

typedef struct ctr_proto {
    uint32_t            id;
    char*               name;             /* debug name, may be NULL        */
    struct ctr_tnode*   ast;              /* source AST                     */
    struct ctr_proto*   parent;           /* lexical parent (NULL for top)  */

    /* frame layout                                                         */
    uint16_t            n_args;
    uint16_t            n_locals;
    uint16_t            n_upvars;
    uint16_t            n_regs;
    uint16_t            local_base;       /* = 1 + n_args                   */
    uint16_t            reg_base;         /* = local_base + n_locals        */

    /* literal tables                                                       */
    double*             nums;             uint32_t n_nums;
    struct ctr_str_lit* strs;             uint32_t n_strs;

    /* references                                                           */
    struct ctr_proto**  protos;           uint32_t n_protos;
    struct ctr_tnode**  asts;             uint32_t n_asts;  /* WALK_AST etc */

    /* call-site caches                                                     */
    ctr_ic_slot*        ics;              uint32_t n_ics;

    ctr_upvar_desc*     upvar_desc;       /* size = n_upvars                */

    /* code                                                                 */
    ctr_instr*          code;             uint32_t n_code;

    /* trace metadata                                                       */
    ctr_exit_record*    exits;            uint32_t n_exits;
    uint32_t            trip_counter;
    unsigned            is_trace : 1;
    unsigned            is_program : 1;   /* top-level program proto        */
    unsigned            promotable : 1;   /* AST escape-free; slot-promoted */
} ctr_proto;

ctr_proto* ctr_bcgen_program(struct ctr_tnode* program);
ctr_proto* ctr_bcgen_block(struct ctr_tnode* codeblock, ctr_proto* parent);
struct ctr_object* ctr_vm_run_program(ctr_proto* p);
struct ctr_object* ctr_vm_run_block(struct ctr_object* block_obj, struct ctr_argument* args, struct ctr_object* receiver_my);
struct ctr_object* ctr_vm_run_proto(ctr_proto* p, struct ctr_object* my, struct ctr_object* myself);
ctr_proto* ctr_proto_for_ast(struct ctr_tnode* ast);
void ctr_proto_register(struct ctr_tnode* ast, ctr_proto* p);
void ctr_proto_dump(ctr_proto* p, int indent);

extern int ctr_use_vm;

#endif /* CTR_BYTECODE_H */
