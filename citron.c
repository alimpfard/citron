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
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void ctr_cli_welcome() {
    printf("\n");
    printf("CTR Programming Language V " CTR_VERSION "\n");
    printf("Written by AnotherTest (c) copyright 2017, Licensed BSD.\n");
    printf("\tExtensions at: " CTR_STD_EXTENSION_PATH "\n");
    printf("\n");
}

/**
 * CommandLine Read Arguments
 * Parses command line arguments and sets global settings accordingly.
 */
void ctr_cli_read_args(int argc, char* argv[]) {
    if (argc == 1) {
        ctr_cli_welcome();
        exit(0);
    }
      ctr_mode_input_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
      strncpy(ctr_mode_input_file, argv[1], 254);
}

/**
 * Citron Application Main Start
 * Bootstraps the Citron Application.
 *
 */
int main(int argc, char* argv[]) {
    volatile char* prg;
    volatile ctr_tnode* program;
    uint64_t program_text_size = 0;

    //pragma rules
    ctr_code_pragma o = { .type = 't', .value = 0 },
                    f = { .type = 'o', .value = 0 },
                    r = { .type = 't', .value = 0 };
    oneLineExpressions = &o;
    flexibleConstructs = &f;
    regexLineCheck = &r;

    ctr_gc_mode = 1; /* default GC mode: activate GC */
    ctr_argc = argc;
    ctr_argv = argv;
    ctr_gc_memlimit = 8388608;
    CTR_LIMIT_MEM = 1; //enfore GC
    ctr_callstack_index = 0;
    ctr_source_map_head = NULL;
    ctr_source_mapping = 0;
    CtrStdFlow = NULL;
    ctr_command_security_profile = 0;
    ctr_command_tick = 0;
    ctr_cli_read_args(argc, argv);
    ctr_source_mapping = 1;
    ctr_clex_keyword_me = CTR_DICT_ME;
    ctr_clex_keyword_my = CTR_DICT_MY;
    ctr_clex_keyword_var = CTR_DICT_VAR;
    ctr_clex_keyword_const = CTR_DICT_CONST;
    ctr_clex_keyword_my_len = strlen( ctr_clex_keyword_my );
    ctr_clex_keyword_var_len = strlen( ctr_clex_keyword_var );
    ctr_clex_keyword_const_len = strlen( ctr_clex_keyword_const );
    ctr_internal_next_return = 0;
    if (ctr_mode_input_file != NULL) {
      prg = ctr_internal_readf(ctr_mode_input_file, &program_text_size);
      program = ctr_cparse_parse(prg, ctr_mode_input_file);
      //ctr_internal_debug_tree(program,1); /*-- for debugging */
      ctr_initialize_world();
      ctr_cwlk_run(program);
    }
    ctr_gc_sweep(1);
    ctr_heap_free( prg );
    ctr_heap_free_rest();
    //For memory profiling
    if ( ctr_gc_alloc != 0 && (CTR_LOG_WARNINGS&1) == 1) {
        printf( "[WARNING] Citron has detected an internal memory leak of: %" PRIu64 " bytes.\n", ctr_gc_alloc );
        exit(1);
    }
    //exit(0);
    return 0;
}
void initialize(int extensions) {
    //pragma rules
    ctr_code_pragma o = { .type = 't', .value = 0 },
                    f = { .type = 'o', .value = 0 },
                    r = { .type = 't', .value = 0 };
    oneLineExpressions = &o;
    flexibleConstructs = &f;
    regexLineCheck = &r;

    ctr_gc_mode = 1; /* default GC mode: activate GC */
    ctr_gc_memlimit = 32 * 1024 * 1024; // 32 MB
    CTR_LIMIT_MEM = 1; //enfore GC
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
    ctr_mode_input_file = ctr_heap_allocate(sizeof(char)*4);
    memcpy(ctr_mode_input_file, "lib", 3);
    *(ctr_mode_input_file+3) = '\0';
    ctr_initialize_world();
    if(extensions) {
      ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
      args->object = ctr_build_string_from_cstring(CTR_STD_EXTENSION_PATH "/extensions/fileutils.ctr");
      ctr_object* futi = ctr_send_message(CtrStdFile, "new:", 4, args);
      ctr_heap_free(args);
      ctr_file_include(futi, NULL);
    }
}

char* execute_string(char* prg) {
    ctr_object* str = ctr_build_string_from_cstring(prg);
    str = ctr_string_eval(str, NULL);
    //ctr_internal_object_set_property(CtrStdWorld, ctr_build_string_from_cstring("_"), str, 0);
    if (CtrStdFlow != NULL) {str = CtrStdFlow; CtrStdFlow = NULL;}
    char* msg = ctr_internal_cast2string(str)->value.svalue->value;
    return msg;
}

char* execute_string_len(char* prg, int length) {
    ctr_object* str = ctr_build_string(prg, length);
    str = ctr_string_eval(str, NULL);
    //ctr_internal_object_set_property(CtrStdWorld, ctr_build_string_from_cstring("_"), str, 0);
    if (CtrStdFlow != NULL) {str = CtrStdFlow; CtrStdFlow = NULL;}
    char* msg = ctr_internal_cast2string(str)->value.svalue->value;
    return msg;
}
