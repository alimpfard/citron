#pragma once

#define DEF_OP_BINARY_INFIX_TY(type, name, op) extern "C" type name (type arg0, type arg1) {\
    return arg0 op arg1;\
}
#define DEF_CALL_BINARY_PREFIXR_TY(type, name, fn) extern "C" type name (type arg0, type arg1) {\
    return fn(arg0, arg1);\
}

#define DEC_OP_BINARY_INFIX_TY(type, name) extern "C" type name (type, type);
#define DEC_CALL_BINARY_PREFIXR_TY(type, name) extern "C" type name (type, type);

DEC_OP_BINARY_INFIX_TY(double, mplus_num);
DEC_OP_BINARY_INFIX_TY(double, msub_num);
DEC_OP_BINARY_INFIX_TY(double, mmul_num);
DEC_OP_BINARY_INFIX_TY(double, mdiv_num);

DEC_CALL_BINARY_PREFIXR_TY(double, mpow_num);

extern "C" double mdummy_num(double, double, double);