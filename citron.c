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
#include <syslog.h>
#include "citron.h"
#include "siphash.h"

#define DO_PROFILE 0
#define CTR_STD_EXTENSION_ENV_NAME "CITRON_EXT_PATH"

#if DO_PROFILE
#include <gperftools/profiler.h>
#endif

static int compile_and_quit = 0;
static int debug = 0;
static int from_stdin = 0;
int with_stdlib = 1;
/**
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void
ctr_cli_welcome (char *invoked_by)
{
  printf ("\n");
  printf ("CTR Programming Language V " CTR_VERSION "\n");
  printf ("Written by AnotherTest (c) copyright 2017, Licensed BSD.\n");
  printf ("\tbuilt with Extensions at: %s\n", ctr_file_stdext_path_raw ());
  printf ("Usage: %s [options] filename\n", invoked_by);
  puts ("Options:");
  puts ("\t-c | --compile : serialize AST to stdout and exit");
  puts ("\t-fc | --from-compiled : assume file is a serialized AST, execute that");
  puts ("\t-d | enable debug mode");
  puts ("\t-e | read from stdin");
  puts ("\t--no-std | launch without the stdlib");
  puts ("\t--ext | print ext path and exit");
  printf ("\n");
}

void
ctr_question_intent (void)
{
  puts ("What is it that you meant to do?");
  ctr_cli_welcome ("<exec>");
}

static char *EXT_PATH = 0;

char const *
ctr_file_stdext_path_raw ()
{
  if (EXT_PATH)
    return EXT_PATH;
  char *env = getenv (CTR_STD_EXTENSION_ENV_NAME);
#ifdef DEBUG_BUILD
  printf ("%s = %p(%s)\n", CTR_STD_EXTENSION_ENV_NAME, env, env == 0 ? "()" : env);
#endif
  if (env)
    {
      EXT_PATH = env;
      return (char const *) env;
    }
  EXT_PATH = CTR_STD_EXTENSION_PATH;
  return CTR_STD_EXTENSION_PATH;
}

/**
 * CommandLine Read Arguments
 * Parses command line arguments and sets global settings accordingly.
 */
void
ctr_cli_read_args (int argc, char *argv[])
{
  if (argc == 1 || argv == NULL)
    {
      ctr_cli_welcome (argv[0]);
      exit (0);
    }
  argc--;
  argv++;
  while (argc > 0 && argv && argv[0][0] == '-')
    {
      if (strcmp (argv[0], "-c") == 0 || strcmp (argv[0], "--compile") == 0)
	compile_and_quit = 1;
      else if (strcmp (argv[0], "-fc") == 0 || strcmp (argv[0], "--from-compiled") == 0)
	compile_and_quit = 2;
      else if (strcmp (argv[0], "-d") == 0)
	debug = 1;
      else if (strcmp (argv[0], "-e") == 0)
	from_stdin = 1;
      else if (strcmp (argv[0], "--ext") == 0)
	{
	  puts (ctr_file_stdext_path_raw ());
	  exit (0);
	}
      else if (strcmp (argv[0], "--no-std") == 0)
	with_stdlib = 0;
      else
	break;
      argv++;
      argc--;
    }
  if (!from_stdin && argv[0] == NULL)
    {
      ctr_question_intent ();
      exit (0);
    }
  ctr_mode_input_file = (char *) ctr_heap_allocate_tracked (sizeof (char) * 255);
  strncpy (ctr_mode_input_file, from_stdin ? "<stdin>" : argv[0], 254);
}

#ifndef CITRON_LIBRARY
/**
 * Citron Application Main Start
 * Bootstraps the Citron Application.
 *
 */
int
main (int argc, char *argv[])
{
  char *prg;
  ctr_tnode *program;
  uint64_t program_text_size = 0;

  //pragma rules
  ctr_code_pragma o = {.type = 't',.value = 0 }, f =
  {
  .type = 'o',.value = 0}, r =
  {
  .type = 't',.value = 0};
  oneLineExpressions = &o;
  flexibleConstructs = &f;
  regexLineCheck = &r;

  ctr_gc_mode = 1;		/* default GC mode: activate GC */
  ctr_argc = argc;
  ctr_argv = argv;
  ctr_gc_memlimit = 8388608;
  CTR_LIMIT_MEM = 1;		//enfore GC
  ctr_callstack_index = 0;
  ctr_source_map_head = NULL;
  ctr_source_mapping = 0;
  CtrStdFlow = NULL;
  ctr_command_security_profile = 0;
  ctr_command_tick = 0;
  ctr_cli_read_args (argc, argv);
  ctr_source_mapping = 1;
  ctr_clex_keyword_me = CTR_DICT_ME;
  ctr_clex_keyword_my = CTR_DICT_MY;
  ctr_clex_keyword_var = CTR_DICT_VAR;
  ctr_clex_keyword_const = CTR_DICT_CONST;
  ctr_clex_keyword_my_len = strlen (ctr_clex_keyword_my);
  ctr_clex_keyword_var_len = strlen (ctr_clex_keyword_var);
  ctr_clex_keyword_const_len = strlen (ctr_clex_keyword_const);
  ctr_internal_next_return = 0;
  ctr_clex_quiet = 0;
  ctr_cparse_quiet = 0;
  ctr_cparse_calltime_name_id = -1;
  openlog (argv[0], LOG_PID | LOG_CONS, LOG_USER);
  ctr_initialize_world ();

#if (DO_PROFILE)
  ProfilerStart ("citron.log");
#endif

  if (ctr_mode_input_file != NULL)
    {
      if (from_stdin)
	prg = ctr_internal_readfp (stdin, &program_text_size);
      else
	prg = ctr_internal_readf (ctr_mode_input_file, &program_text_size);
      if (compile_and_quit == 2)
	{
	  ctr_size len = 0;
	  program = ctr_unmarshal_ast (prg, program_text_size, &len);
	}
      else
	program = ctr_cparse_parse (prg, ctr_mode_input_file);
      if (compile_and_quit == 1)
	{
	  ctr_size blen = 102400, bused = 0;
	  char *bufp = ctr_heap_allocate (blen);
	  ctr_marshal_ast (program, &bufp, &blen, &bused);
	  fwrite (bufp, 1, bused, stdout);
	  exit (0);
	}
#ifdef comp
      ctr_ccomp_env_update_new_context ();
      printf ("%d\n", ctr_ccomp_node_indeterministic_level (program));
      ctr_ccomp_env_update_pop_context ();
      printf ("\n\nBEFORE OPTIMIZE\n");
      ctr_internal_debug_tree (program, 1);
	/*-- for debugging */
      ctr_ccomp_optimize_node_inplace (&program);
      printf ("\n\nAFTER OPTIMIZE\n");
      ctr_internal_debug_tree (program, 1);
	/*-- for debugging */
      // ctr_initialize_world ();
      // ctr_ccomp_run (program);

      closelog ();
      exit (0);
#endif
      if (debug)
	ctr_internal_debug_tree (program, 1);	      /*-- for debugging */
      ctr_cwlk_run (program);
      ctr_gc_sweep (1);
      ctr_heap_free (prg);
      ctr_heap_free_rest ();
      //For memory profiling
      if (ctr_gc_alloc != 0 && (CTR_LOG_WARNINGS & 1) == 1)
	{
	  printf ("[WARNING] Citron has detected an internal memory leak of: %" PRIu64 " bytes.\n", ctr_gc_alloc);
	  closelog ();
	  exit (1);
	}
      //exit(0);
#if (DO_PROFILE)
      ProfilerStop ();
#endif
    }
  return 0;
}
#endif //CITRON_LIBRARY
void
ctr_initialize_ex (int heap_length, int extensions)
{
  //pragma rules
  ctr_code_pragma o = {.type = 't',.value = 0 }, f =
  {
  .type = 'o',.value = 0}, r =
  {
  .type = 't',.value = 0};
  oneLineExpressions = &o;
  flexibleConstructs = &f;
  regexLineCheck = &r;

  ctr_gc_mode = 1;		/* default GC mode: activate GC */
  ctr_gc_memlimit = heap_length;	// 32 MB
  CTR_LIMIT_MEM = 1;		//enfore GC
  ctr_callstack_index = 0;
  ctr_source_map_head = NULL;
  ctr_source_mapping = 0;
  CtrStdFlow = NULL;
  ctr_command_security_profile = 0;
  ctr_command_tick = 0;
  ctr_source_mapping = 1;
  ctr_clex_keyword_me = strdup(CTR_DICT_ME);
  ctr_clex_keyword_my = strdup(CTR_DICT_MY);
  ctr_clex_keyword_var = strdup(CTR_DICT_VAR);
  ctr_clex_keyword_const = strdup(CTR_DICT_CONST);
  ctr_clex_keyword_my_len = strlen (ctr_clex_keyword_my);
  ctr_clex_keyword_var_len = strlen (ctr_clex_keyword_var);
  ctr_clex_keyword_const_len = strlen (ctr_clex_keyword_const);
  ctr_mode_input_file = ctr_heap_allocate (sizeof (char) * 4);
  ctr_clex_quiet = 0;
  ctr_cparse_quiet = 0;
  memcpy (ctr_mode_input_file, "lib", 3);
  *(ctr_mode_input_file + 3) = '\0';
  ctr_initialize_world ();
  if (extensions && with_stdlib)
    {
      ctr_argument *args = ctr_heap_allocate (sizeof (ctr_argument));
      const char *epath = ctr_file_stdext_path_raw ();
      static char path_[2048];
      size_t len = sprintf (path_, "%s/extensions/fileutils.ctr", epath);
      args->object = ctr_build_string (path_, len);
      ctr_object *futi = ctr_send_message (CtrStdFile, "new:", 4, args);
      ctr_heap_free (args);
      ctr_file_include (futi, NULL);
    }
}

char *
execute_string (char *prg)
{
  ctr_object *str = ctr_build_string_from_cstring (prg);
  str = ctr_string_eval (str, NULL);
  //ctr_internal_object_set_property(CtrStdWorld, &CTR_CLEX_US, str, 0);
  if (CtrStdFlow != NULL)
    {
      str = CtrStdFlow;
      CtrStdFlow = NULL;
    }
  char *msg = ctr_internal_cast2string (str)->value.svalue->value;
  return msg;
}

char *
execute_string_len (char *prg, int length)
{
  ctr_object *str = ctr_build_string (prg, length);
  str = ctr_string_eval (str, NULL);
  //ctr_internal_object_set_property(CtrStdWorld, &CTR_CLEX_US, str, 0);
  if (CtrStdFlow != NULL)
    {
      str = CtrStdFlow;
      CtrStdFlow = NULL;
    }
  char *msg = ctr_internal_cast2string (str)->value.svalue->value;
  return msg;
}
