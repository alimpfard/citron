#include <inttypes.h>

#include "citron.h"
#include "siphash.h"
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define DO_PROFILE 0
#define CTR_STD_EXTENSION_ENV_NAME "CITRON_EXT_PATH"

#if DO_PROFILE
#include <gperftools/profiler.h>
#endif

int ctr_did_side_effect = 0;

static int compile_and_quit = 0;
static int debug = 0;
static int from_stdin = 0;
int with_stdlib = 1;
static int parse_only = 0;
extern int speculative_parse;
extern int more_exception_data;

char *SystemTZ;

ctr_code_pragma oneline_p = {.type = 't', .value = 0},
                flex_const =

                    {.type = 'o', .value = 0},
                regex_lc =

                    {.type = 't', .value = 0},
                callshorth =

                    {.type = 'o',
                     .value = CTR_TOKEN_TUPOPEN,
                     .value_e = CTR_TOKEN_TUPCLOSE},
                extpragmas =

                    {.type = 'o', .value = 0},
                nextlazy =

                    {.type = 't', .value = 0};

/**
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void ctr_cli_welcome(char *invoked_by) {
  printf("\n");
  printf("CTR Programming Language V " CTR_VERSION "\n");
  printf("Written by AnotherTest (c) copyright 2017, Licensed BSD.\n");
  printf("\tbuilt with Extensions at: %s\n", ctr_file_stdext_path_raw());
  printf("Usage: %s [options] filename\n", invoked_by);
  puts("Options:");
  puts("\t-s | enable speculative parsing");
  puts("\t-p | parse and exit");
  puts("\t-c | --compile : serialize AST to stdout and exit");
  puts("\t-fc | --from-compiled : assume file is a serialized AST, execute "
       "that");
  puts("\t-d | enable debug mode");
  puts("\t-- | read from stdin");
  puts("\t--no-std | launch without the stdlib");
  puts("\t--ext | print ext path and exit");
  puts("\t--compact | display no extra data for exceptions");
  printf("\n");
}

void ctr_question_intent(void) {
  puts("What is it that you meant to do?");
  ctr_cli_welcome("<exec>");
}

static char *EXT_PATH = 0;
/*static char *argv0 = 0;
static char abs_exe_path[PATH_MAX];
char* applicationDirPath(char* argv0) {
  char path_save[PATH_MAX];
  char *p;

  if((p = strrchr(argv0, '/')))
    getcwd(abs_exe_path, sizeof(abs_exe_path));
  else
  {
    *p = '\0';
    getcwd(path_save, sizeof(path_save));
    chdir(argv0);
    chdir("../share/Citron");
    getcwd(abs_exe_path, sizeof(abs_exe_path));
    chdir(path_save);
  }
  return abs_exe_path;
}*/

char const *ctr_file_stdext_path_raw() {
  if (EXT_PATH)
    return EXT_PATH;
  char *env = getenv(CTR_STD_EXTENSION_ENV_NAME);
#ifdef DEBUG_BUILD
  fprintf(stderr, "%s = %p(%s)\n", CTR_STD_EXTENSION_ENV_NAME, env,
         env == 0 ? "()" : env);
#endif
  if (env) {
    EXT_PATH = env;
    return (char const *)env;
  }
  EXT_PATH = CTR_STD_EXTENSION_PATH; // applicationDirPath(argv0);
  return EXT_PATH;
}

/**
 * CommandLine Read Arguments
 * Parses command line arguments and sets global settings accordingly.
 */
void ctr_cli_read_args(int argc, char *argv[]) {
  if (argc == 1 || argv == NULL) {
    ctr_cli_welcome(argv[0]);
    exit(0);
  }
  argc--;
  argv++;
  while (argc > 0 && argv && argv[0][0] == '-') {
    if (strcmp(argv[0], "-c") == 0 || strcmp(argv[0], "--compile") == 0)
      compile_and_quit = 1;
    else if (strcmp(argv[0], "-fc") == 0 ||
             strcmp(argv[0], "--from-compiled") == 0)
      compile_and_quit = 2;
    else if (strcmp(argv[0], "-p") == 0)
      parse_only = 1;
    else if (strcmp(argv[0], "-s") == 0)
      speculative_parse = 1;
    else if (strcmp(argv[0], "-d") == 0)
      debug = 1;
    else if (strcmp(argv[0], "--") == 0)
      from_stdin = 1;
    else if (strcmp(argv[0], "--compact") == 0)
      more_exception_data = 0;
    else if (strcmp(argv[0], "--ext") == 0) {
      puts(ctr_file_stdext_path_raw());
      exit(0);
    } else if (strcmp(argv[0], "--no-std") == 0)
      with_stdlib = 0;
    else
      break;
    argv++;
    argc--;
  }
  if (!from_stdin && argv[0] == NULL) {
    ctr_question_intent();
    exit(0);
  }
  ctr_mode_input_file = (char *)ctr_heap_allocate_tracked(sizeof(char) * 255);
  strncpy(ctr_mode_input_file, from_stdin ? "<stdin>" : argv[0], 254);
}

static const char ME_S[] = CTR_DICT_ME, MY_S[] = CTR_DICT_MY,
                  VAR_S[] = CTR_DICT_VAR, CONST_S[] = CTR_DICT_CONST,
                  STATIC_S[] = CTR_DICT_STATIC;

void lambdaf_begin_instance();
void lambdaf_end_instance();

void ctr_initialize_ex() {
  lambdaf_begin_instance();
  // GC_enable_incremental();
  oneLineExpressions = &oneline_p;
  flexibleConstructs = &flex_const;
  regexLineCheck = &regex_lc;
  callShorthand = &callshorth;
  extensionsPra = &extpragmas;
  nextCallLazy = &nextlazy;

  SystemTZ = getenv("TZ") ?: "UTC";

  ctr_gc_mode = 1; /* default GC mode: activate GC */
  ctr_gc_memlimit = 1073741824;
  CTR_LIMIT_MEM = 1; // enfore GC
  ctr_callstack_index = 0;
  ctr_source_map_head = NULL;
  ctr_source_mapping = 0;
  CtrStdFlow = NULL;
  ctr_command_security_profile = 0;
  ctr_command_tick = 0;
  ctr_source_mapping = 1;
  ctr_clex_keyword_me = (char *)ME_S;
  ctr_clex_keyword_my = (char *)MY_S;
  ctr_clex_keyword_var = (char *)VAR_S;
  ctr_clex_keyword_const = (char *)CONST_S;
  ctr_clex_keyword_static = (char *)STATIC_S;
  ctr_clex_keyword_my_len = strlen(MY_S);
  ctr_clex_keyword_var_len = strlen(VAR_S);
  ctr_clex_keyword_const_len = strlen(CONST_S);
  ctr_clex_keyword_static_len = strlen(STATIC_S);
  ctr_internal_next_return = 0;
  ctr_clex_quiet = 0;
  ctr_cparse_quiet = 0;
  ctr_cparse_calltime_name_id = -1;
  ctr_initialize_world();
}

#ifndef LIBRARY_BUILD
#ifndef CITRON_LIBRARY
/**
 * Citron Application Main Start
 * Bootstraps the Citron Application.
 *
 */
int main(int argc, char *argv[]) {
  char *prg;
  ctr_tnode *program;
  uint64_t program_text_size = 0;
  // argv0 = argv[0];
  ctr_argc = argc;
  ctr_argv = argv;
  ctr_cli_read_args(argc, argv);
  ctr_initialize_ex();
  openlog(argv[0], LOG_PID | LOG_CONS, LOG_USER);
#if (DO_PROFILE)
  ProfilerStart("citron.log");
#endif

  if (ctr_mode_input_file != NULL) {
    if (from_stdin)
      prg = ctr_internal_readfp(stdin, &program_text_size);
    else
      prg = ctr_internal_readf(ctr_mode_input_file, &program_text_size);
    if (compile_and_quit == 2) {
      ctr_size len = 0;
      program = ctr_unmarshal_ast(prg, program_text_size, &len);
    } else
      program = ctr_cparse_parse(prg, ctr_mode_input_file);
    if (compile_and_quit == 1) {
      ctr_size blen = 102400, bused = 0;
      char *bufp = ctr_heap_allocate(blen);
      ctr_marshal_ast(program, &bufp, &blen, &bused);
      fwrite(bufp, 1, bused, stdout);
      exit(0);
    }
#ifdef comp
    ctr_ccomp_env_update_new_context();
    printf("%d\n", ctr_ccomp_node_indeterministic_level(program));
    ctr_ccomp_env_update_pop_context();
    printf("\n\nBEFORE OPTIMIZE\n");
    ctr_internal_debug_tree(program, 1);
    /*-- for debugging */
    ctr_ccomp_optimize_node_inplace(&program);
    printf("\n\nAFTER OPTIMIZE\n");
    ctr_internal_debug_tree(program, 1);
    /*-- for debugging */
    // ctr_initialize_world ();
    // ctr_ccomp_run (program);

    closelog();
    exit(0);
#endif
    if (debug)
      ctr_internal_debug_tree(program, 1); /*-- for debugging */
    if (!parse_only)
      ctr_cwlk_run(program);
    ctr_gc_sweep(1);
    ctr_heap_free(prg);
    ctr_heap_free_rest();
    // For memory profiling
    if (ctr_gc_alloc != 0 && (CTR_LOG_WARNINGS & 1) == 1) {
      printf(
          "[WARNING] Citron has detected an internal memory leak of: %" PRIu64
          " bytes.\n",
          ctr_gc_alloc);
      closelog();
      exit(1);
    }
    // exit(0);
#if (DO_PROFILE)
    ProfilerStop();
#endif
  }
  lambdaf_end_instance();
  return 0;
}
#endif // CITRON_LIBRARY
void initialize(int extensions) {
  ctr_initialize_ex();
  memcpy(ctr_mode_input_file, "lib", 3);
  *(ctr_mode_input_file + 3) = '\0';
  ctr_initialize_world();
  if (extensions && with_stdlib) {
    ctr_argument *args = ctr_heap_allocate(sizeof(ctr_argument));
    const char *epath = ctr_file_stdext_path_raw();
    static char path_[2048];
    size_t len = sprintf(path_, "%s/extensions/fileutils.ctr", epath);
    args->object = ctr_build_string(path_, len);
    ctr_object *futi = ctr_send_message(CtrStdFile, "new:", 4, args);
    ctr_heap_free(args);
    ctr_file_include(futi, NULL);
  }
}

char *execute_string(char *prg) {
  ctr_object *str = ctr_build_string_from_cstring(prg);
  str = ctr_string_eval(str, NULL);
  // ctr_internal_object_set_property(CtrStdWorld, &CTR_CLEX_US, str, 0);
  if (CtrStdFlow != NULL) {
    str = CtrStdFlow;
    CtrStdFlow = NULL;
  }
  char *msg = ctr_internal_cast2string(str)->value.svalue->value;
  return msg;
}

char *execute_string_len(char *prg, int length) {
  ctr_object *str = ctr_build_string(prg, length);
  str = ctr_string_eval(str, NULL);
  // ctr_internal_object_set_property(CtrStdWorld, &CTR_CLEX_US, str, 0);
  if (CtrStdFlow != NULL) {
    str = CtrStdFlow;
    CtrStdFlow = NULL;
  }
  char *msg = ctr_internal_cast2string(str)->value.svalue->value;
  return msg;
}
#endif // LIBRARY_BUILD
