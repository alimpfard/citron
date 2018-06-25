#include <cmath>
#include "fns.hpp"

DEF_OP_BINARY_INFIX_TY(double, mplus_num, +);
DEF_OP_BINARY_INFIX_TY(double, msub_num, -);
DEF_OP_BINARY_INFIX_TY(double, mmul_num, *);
DEF_OP_BINARY_INFIX_TY(double, mdiv_num, /);

DEF_CALL_BINARY_PREFIXR_TY(double, mpow_num, pow);

extern "C" double mdummy_num(double a, double b, double c) {
    return a + b + c;
}