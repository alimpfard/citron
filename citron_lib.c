#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include "citron.h"
#include "siphash.h"

/**
 *
 * Initialize the world. MUST be called first.
 *
 */
static void initialize() {
    //pragma rules
    ctr_code_pragma o = { .type = 't', .value = 0 },
                    f = { .type = 'o', .value = 0 },
                    r = { .type = 't', .value = 0 };
    oneLineExpressions = &o;
    flexibleConstructs = &f;
    regexLineCheck = &r;

    ctr_gc_mode = 1; /* default GC mode: activate GC */
    ctr_gc_memlimit = 8388608;
    ctr_callstack_index = 0;
    ctr_source_map_head = NULL;
    ctr_source_mapping = 0;
    CtrStdFlow = NULL;
    ctr_command_security_profile = 0;
    ctr_command_tick = 0;
    ctr_source_mapping = 1;
    ctr_clex_keyword_me = CTR_DICT_ME;
    ctr_clex_keyword_my = CTR_DICT_MY;
    ctr_clex_keyword_var = CTR_DICT_VAR;
    ctr_clex_keyword_const = CTR_DICT_CONST;
    ctr_clex_keyword_my_len = strlen( ctr_clex_keyword_my );
    ctr_clex_keyword_var_len = strlen( ctr_clex_keyword_var );
    ctr_clex_keyword_const_len = strlen( ctr_clex_keyword_const );
    ctr_initialize_world();
}

static int execute_string(char* prg) {
    ctr_tnode* program;
    program = ctr_cparse_parse(prg, ctr_mode_input_file);
    ctr_cwlk_run(program);
    ctr_gc_sweep(1);
    ctr_heap_free( prg );
    ctr_heap_free_rest();
    //For memory profiling
    if ( ctr_gc_alloc != 0 ) {
        return ctr_gc_alloc;
    }
    return 0;
}
