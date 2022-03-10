#pragma once

enum AsmArgType {
    ASM_ARG_TY_INT,
    ASM_ARG_TY_DBL,
    ASM_ARG_TY_RES,
    ASM_ARG_TY_STR,
    ASM_ARG_TY_PTR,
};
typedef struct AsmArgInfo {
    enum AsmArgType ty;

} asm_arg_info_t;

extern void* ctr_cparse_intern_asm_block(char const* asm_begin, char const* asm_end, char const* con, int offset, int argc, asm_arg_info_t* arginfo, int att);
