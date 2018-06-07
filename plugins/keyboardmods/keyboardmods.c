#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#ifdef existing
#include <Citron/citron.h>
#else
#warning "We don't have Citron installed"
#include "../../citron.h"
#endif
#define KG_SHIFT        0
#define KG_CTRL         2
#define KG_ALT          3
#define KG_ALTGR        1
#define KG_SHIFTL       4
#define KG_KANASHIFT    4
#define KG_SHIFTR       5
#define KG_CTRLL        6
#define KG_CTRLR        7
#define KG_CAPSSHIFT    8

static ctr_argument carg = {.object = NULL, .next = NULL};

ctr_object* interpret(char mod) {
  ctr_object* descr = ctr_array_new(CtrStdArray, NULL);
  if (mod&KG_SHIFT) {
    carg.object = ctr_get_or_create_symbol_table_entry("SHIFT", 5);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_CTRL) {
    carg.object = ctr_get_or_create_symbol_table_entry("CTRL", 4);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_ALT) {
    carg.object = ctr_get_or_create_symbol_table_entry("ALT", 3);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_ALTGR) {
    carg.object = ctr_get_or_create_symbol_table_entry("ALTGR", 5);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_SHIFTL) {
    carg.object = ctr_get_or_create_symbol_table_entry("SHIFTL", 6);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_KANASHIFT) {
    carg.object = ctr_get_or_create_symbol_table_entry("KANASHIFT", 9);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_SHIFTR) {
    carg.object = ctr_get_or_create_symbol_table_entry("SHIFTR", 6);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_CTRLL) {
    carg.object = ctr_get_or_create_symbol_table_entry("CTRLL", 5);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_CTRLR) {
    carg.object = ctr_get_or_create_symbol_table_entry("CTRLR", 5);
    ctr_array_push(descr, &carg);
  }
  if (mod&KG_CAPSSHIFT) {
    carg.object = ctr_get_or_create_symbol_table_entry("CAPSSHIFT", 9);
    ctr_array_push(descr, &carg);
  }
  return descr;
}

ctr_object* check_for_modifier(ctr_object* myself, ctr_argument* argumentList)
{
    static char shift_state;
    shift_state = 6;
    if (ioctl(0, TIOCLINUX, &shift_state) < 0) {
            perror("ioctl TIOCLINUX 6 (get shift state)");
            CtrStdFlow = ctr_build_string_from_cstring("IOCTL TIOCLINUX 6 failed");
            return CtrStdNil;
    }
    return interpret(shift_state);
}

void begin() {
  ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring("shiftState"), &check_for_modifier);
}
