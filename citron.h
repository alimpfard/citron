#ifndef CTR_H_GUARD
#define CTR_H_GUARD

#include "config.h"

char const* ctr_file_stdext_path_raw();

#ifdef  __cplusplus

#define CTR_H_DECLSPEC extern
extern "C" {

#else //__cplusplus

#define CTR_H_DECLSPEC

#endif

#define CTR_GLOBAL_SPEC extern

#define CTR_LOAD_BASE_MODS //undef for no base mods

#include "dictionary.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DEBUG_BUILD
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)
#pragma message VAR_NAME_VALUE(DEBUG_BUILD_VERSION)
#define IS_DEBUG_STRING "-debug" DEBUG_BUILD_VERSION
#else
#define IS_DEBUG_STRING ""
#endif

// whether to use tcc as inject core
// if undefined, Inject will not be available
#ifndef withInjectNative
#define withInjectNative 0
#endif

// whether to include CTypes / ffi
#ifndef withCTypesNative
#define withCTypesNative 0
#endif

#ifdef withBoehmGC
#define CTR_VERSION "0.0.9.1-boehm-gc" IS_DEBUG_STRING
#else
#define CTR_VERSION "0.0.9.1" IS_DEBUG_STRING
#endif

#define CTR_LOG_WARNINGS 2//2 to enable
/**
 * Define the Citron tokens
 */
#define CTR_TOKEN_REF 1
#define CTR_TOKEN_QUOTE 2
#define CTR_TOKEN_NUMBER 3
#define CTR_TOKEN_PAROPEN 4
#define CTR_TOKEN_PARCLOSE 5
#define CTR_TOKEN_BLOCKOPEN 6
#define CTR_TOKEN_BLOCKOPEN_MAP 7
#define CTR_TOKEN_BLOCKCLOSE 8
#define CTR_TOKEN_COLON 9
#define CTR_TOKEN_DOT 10
#define CTR_TOKEN_CHAIN 11
#define CTR_TOKEN_BOOLEANYES 13
#define CTR_TOKEN_BOOLEANNO 14
#define CTR_TOKEN_NIL 15
#define CTR_TOKEN_ASSIGNMENT 16
#define CTR_TOKEN_RET 17
#define CTR_TOKEN_TUPOPEN 18
#define CTR_TOKEN_TUPCLOSE 19
#define CTR_TOKEN_PASSIGNMENT 20
#define CTR_TOKEN_SYMBOL 21
#define CTR_TOKEN_LITERAL_ESC 22
#define CTR_TOKEN_INV 23
#define CTR_TOKEN_FANCY_QUOT_OPEN 25
#define CTR_TOKEN_FANCY_QUOT_CLOS 36
#define CTR_TOKEN_FIN 99
//
//
#define EXIT_ON_ERROR //Exit on parse error.
#undef EXIT_ON_ERROR

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/**
 * Define the UTF8 byte patterns
 */
#define CTR_UTF8_BYTE1 192
#define CTR_UTF8_BYTE2 224
#define CTR_UTF8_BYTE3 240

/**
 * Define AST node memory footprints,
 * types of nodes in the AST.
 */
#define CTR_AST_NODE 1
#define CTR_AST_PROGRAM 3
#define CTR_AST_TYPE 5

/**
 * Define the Citron node types for the
 * Abstract Syntax Tree (AST).
 */
#define CTR_AST_NODE_EXPRASSIGNMENT 51
#define CTR_AST_NODE_EXPRMESSAGE 52
#define CTR_AST_NODE_UNAMESSAGE 53
#define CTR_AST_NODE_BINMESSAGE 54
#define CTR_AST_NODE_KWMESSAGE 55
#define CTR_AST_NODE_LTRSTRING 56
#define CTR_AST_NODE_REFERENCE 57
#define CTR_AST_NODE_LTRNUM 58
#define CTR_AST_NODE_CODEBLOCK 59
#define CTR_AST_NODE_RETURNFROMBLOCK 60
#define CTR_AST_NODE_IMMUTABLE 61
#define CTR_AST_NODE_SYMBOL 62
#define CTR_AST_NODE_LISTCOMP 63
#define CTR_AST_NODE_RAW 64
#define CTR_AST_NODE_EMBED 65
#define CTR_AST_NODE_PARAMLIST 76
#define CTR_AST_NODE_INSTRLIST 77
#define CTR_AST_NODE_ENDOFPROGRAM 79
#define CTR_AST_NODE_NESTED 80
#define CTR_AST_NODE_LTRBOOLTRUE 81
#define CTR_AST_NODE_LTRBOOLFALSE 82
#define CTR_AST_NODE_LTRNIL 83
#define CTR_AST_NODE_PROGRAM 84
#define CTR_AST_NODE_NATIVEFN 85

/**
 * Define the basic object types.
 * All objects in Citron are 'normal' objects, however some
 * native objects (and external objects) have special memory requirements,
 * these are specified using the object types.
 */
#define CTR_OBJECT_TYPE_OTNIL 0
#define CTR_OBJECT_TYPE_OTBOOL 1
#define CTR_OBJECT_TYPE_OTNUMBER 2
#define CTR_OBJECT_TYPE_OTSTRING 3
#define CTR_OBJECT_TYPE_OTBLOCK 4
#define CTR_OBJECT_TYPE_OTOBJECT 5
#define CTR_OBJECT_TYPE_OTNATFUNC 6
#define CTR_OBJECT_TYPE_OTARRAY 7
#define CTR_OBJECT_TYPE_OTMISC 8
#define CTR_OBJECT_TYPE_OTEX 9

/**
 * Define the two types of properties of
 * objects.
 */
#define CTR_CATEGORY_PRIVATE_PROPERTY 0
#define CTR_CATEGORY_PUBLIC_PROPERTY 0 /* same, all properties are PRIVATE, except those in CtrStdWorld, this is just to avoid confusion */
#define CTR_CATEGORY_PUBLIC_METHOD 1

/**
 * Security profile bit flags.
 */
#define CTR_SECPRO_NO_SHELL 1
#define CTR_SECPRO_NO_FILE_WRITE 2
#define CTR_SECPRO_NO_FILE_READ 4
#define CTR_SECPRO_NO_INCLUDE 8
#define CTR_SECPRO_COUNTDOWN 16
#define CTR_SECPRO_EVAL 0 //32 to enable
#define CTR_SECPRO_FORK 64

#define CTR_ANSI_COLOR_RED     "\x1b[31m"
#define CTR_ANSI_COLOR_GREEN   "\x1b[32m"
#define CTR_ANSI_COLOR_YELLOW  "\x1b[33m"
#define CTR_ANSI_COLOR_BLUE    "\x1b[34m"
#define CTR_ANSI_COLOR_MAGENTA "\x1b[35m"
#define CTR_ANSI_COLOR_CYAN    "\x1b[36m"
#define CTR_ANSI_COLOR_RESET   "\x1b[0m"

/**
 * Define basic types for Citron
 */
typedef  unsigned int ctr_bool;
typedef  double       ctr_number;
typedef  char*        ctr_raw_string;

typedef  size_t ctr_size;

/**
 * Internal Citron String
 */
struct ctr_string {
	char* value;
	ctr_size vlen;
};
typedef struct ctr_string ctr_string;


/**
 * Map
 */
struct ctr_map {
	struct ctr_mapitem* head;
	int size;
	// uint64_t s_hash;
	// uint32_t m_hash;
};
typedef struct ctr_map ctr_map;

/**
 * Map item
 */
struct ctr_mapitem {
	int hits;
	uint64_t hashKey;
	struct ctr_object* key;
	struct ctr_object* value;
	struct ctr_mapitem* prev;
	struct ctr_mapitem* next;
};
typedef struct ctr_mapitem ctr_mapitem;

/**
 * Citron Argument (internal, not accsible to users).
 */
struct ctr_argument {
	struct ctr_object* object;
	struct ctr_argument* next;
};
typedef struct ctr_argument ctr_argument;
typedef void(*voidptrfn_t(void*));

struct ctr_interfaces {
	int count;
	struct ctr_object* link;
	struct ctr_object** ifs;
};
typedef struct ctr_interfaces ctr_interfaces;

struct ctr_overload_set {
	int bucket_count;
	struct ctr_overload_set** sub_buckets;
	struct ctr_object * this_terminating_value;
	struct ctr_object * this_terminating_bucket;
};

typedef struct ctr_overload_set ctr_overload_set;

/**
 * Root Object
 */
struct ctr_object {
	ctr_map* properties;
	ctr_map* methods;
	struct {
		unsigned int type: 4;
		unsigned int mark: 1;
		unsigned int sticky: 1;
		unsigned int asyncMode: 1;
		unsigned int chainMode: 1;
		unsigned int remote: 1;
		unsigned int shared: 1;
		unsigned int raw: 1;
		unsigned int overloaded: 1;
	} info;
	struct ctr_interfaces* interfaces;
	struct ctr_object* lexical_name;
	union uvalue {
		ctr_bool bvalue;
		ctr_number nvalue;
		ctr_string* svalue;
		struct ctr_tnode* block;
		struct ctr_collection* avalue;
		struct ctr_resource* rvalue;
		struct ctr_object* (*fvalue) (struct ctr_object* myself, struct ctr_argument* argumentList);
		struct ctr_object* defvalue;
	} value;
	union {
		voidptrfn_t* release_hook;
		struct ctr_overload_set*  overloads; // overloaded functions may not specify a release hook
	};
#if !defined(withBoehmGC)
	struct ctr_object* gnext;
#endif
};
typedef struct ctr_object ctr_object;

/**
 * Citron Resource
 */
struct ctr_resource {
	unsigned int type;
	void* ptr;
};
typedef struct ctr_resource ctr_resource;

/**
 * Array Structure
 */
struct ctr_collection {
	int immutable: 1;
	ctr_size length;
	ctr_size head;
	ctr_size tail;
	ctr_object** elements;
};
typedef struct ctr_collection ctr_collection;


/**
 * AST Node
 */
struct ctr_tnode {
	char type;
	char modifier;
	char* value;
	ctr_size vlen;
	struct ctr_tlistitem* nodes;
	unsigned int lexical: 1;
};
typedef struct ctr_tnode ctr_tnode;

/**
 * AST Node List
 */
struct ctr_tlistitem {
	ctr_tnode* node;
	struct ctr_tlistitem* next;
};
typedef struct ctr_tlistitem ctr_tlistitem;

struct ctr_source_map {
	ctr_tnode* node;
	char* p_ptr;
	char* s_ptr;
	char* e_ptr;
	uint32_t line;
	struct ctr_source_map* next;
};
typedef struct ctr_source_map ctr_source_map;

CTR_H_DECLSPEC ctr_source_map* ctr_source_map_head;
CTR_H_DECLSPEC int ctr_source_mapping;

/**
 * General Pragma Structure
 */
struct ctr_code_pragma {
 char type; // t -> toggle, o -> one-shot
 int value;
 int value_e;
};
typedef struct ctr_code_pragma ctr_code_pragma;

struct fixity_lookup_rv {
    int fix;
    int prec;
	int lazy;
};
typedef struct fixity_lookup_rv fixity_lookup_rv;

CTR_H_DECLSPEC int CTR_CCOMP_SIMULATION; //in compiler simulation mode
ctr_object* ctr_ccomp_get_stub(ctr_object*(*nfunc)(ctr_object*,ctr_argument*), ctr_object* receiver, ctr_argument* args); //stub generator lookup table
CTR_H_DECLSPEC ctr_argument* CtrCompilerStub; //returned object in case of stub error
extern int with_stdlib;

/**
 * Core Objects
 */

#ifdef CTR_INJECT
#define __thread
#endif

CTR_H_DECLSPEC ctr_object* CtrStdWorld;          //!< Standard Object : Global Namespace
CTR_H_DECLSPEC ctr_object* CtrStdObject;         //!< Standard Object : Base Object
CTR_H_DECLSPEC ctr_object* CtrStdBlock;          //!< Standard Object : Code Block
CTR_H_DECLSPEC ctr_object* CtrStdString;         //!< Standard Object : String
CTR_H_DECLSPEC ctr_object* CtrStdNumber;         //!< Standard Object : Number
CTR_H_DECLSPEC ctr_object* CtrStdBool;           //!< Standard Object : Boolean
CTR_H_DECLSPEC ctr_object* CtrStdConsole;        //!< Standard Object : Pen
CTR_H_DECLSPEC ctr_object* CtrStdNil;            //!< Standard Object : Nil
CTR_H_DECLSPEC ctr_object* CtrStdGC;             //!< Standard Object : Broom
CTR_H_DECLSPEC ctr_object* CtrStdMap;            //!< Standard Object : Map
CTR_H_DECLSPEC ctr_object* CtrStdHashMap;        //!< Standard Object : HashMap
CTR_H_DECLSPEC ctr_object* CtrStdArray;          //!< Standard Object : Array
CTR_H_DECLSPEC ctr_object* CtrStdIter;           //!< Standard Object : Iterator
CTR_H_DECLSPEC ctr_object* CtrStdFile;           //!< Standard Object : File
CTR_H_DECLSPEC ctr_object* CtrStdSystem;         //!< Standard Object : internal
CTR_H_DECLSPEC ctr_object* CtrStdDice;           //!< Standard Object : Dice
CTR_H_DECLSPEC ctr_object* CtrStdCommand;        //!< Standard Object : Program
CTR_H_DECLSPEC ctr_object* CtrStdSlurp;          //!< Standard Object : Slurp
CTR_H_DECLSPEC ctr_object* CtrStdShell;          //!< Standard Object : Shell
CTR_H_DECLSPEC ctr_object* CtrStdClock;          //!< Standard Object : Clock
#ifndef CTR_GLOBALS_DEFINE
extern __thread ctr_object* CtrStdFlow;
#else
__thread ctr_object* CtrStdFlow = NULL;           //!< Internal Flow namespace : contains errors and flow control
#endif
CTR_H_DECLSPEC ctr_object* CtrExceptionType;     //!< contains error/exception types
CTR_H_DECLSPEC ctr_object* CtrStdBreak;          //!< Internal Flow representation for break : stop the current iteration
CTR_H_DECLSPEC ctr_object* CtrStdContinue;       //!< Internal Flow representation for continue : skip the current iteration
CTR_H_DECLSPEC ctr_object* CtrStdExit;           //!< Internal Flow representation for exit : exit program on this message
CTR_H_DECLSPEC ctr_object* CtrStdReflect;        //!< Standard Object : Reflect
CTR_H_DECLSPEC ctr_object* CtrStdReflect_cons;   //!< Standard Object : cons
CTR_H_DECLSPEC ctr_object* CtrStdFiber;          //!< Standard Object : Fiber
CTR_H_DECLSPEC ctr_object* CtrStdThread;         //!< Standard Object : Thread
CTR_H_DECLSPEC ctr_object* CtrStdSymbol;         //!< Standard Object : Symbol
CTR_H_DECLSPEC ctr_object* ctr_first_object;     //!< Internal Garbage Collector guide object
//--
CTR_H_DECLSPEC ctr_object* CTR_FILE_STDIN;       //!< Special Object : Standard Input File
CTR_H_DECLSPEC ctr_object* CTR_FILE_STDOUT;      //!< Special Object : Standard Output File
CTR_H_DECLSPEC ctr_object* CTR_FILE_STDERR;      //!< Special Object : Standard Error File
CTR_H_DECLSPEC ctr_object* CTR_FILE_STDIN_STR;   //!< Special Object : Standard Input in-world name
CTR_H_DECLSPEC ctr_object* CTR_FILE_STDOUT_STR;  //!< Special Object : Standard Output in-world name
CTR_H_DECLSPEC ctr_object* CTR_FILE_STDERR_STR;  //!< Special Object : Standard Error in-world name

ctr_object** get_CtrStdWorld();
ctr_object** get_CtrStdObject();
ctr_object** get_CtrStdBlock();
ctr_object** get_CtrStdString();
ctr_object** get_CtrStdNumber();
ctr_object** get_CtrStdBool();
ctr_object** get_CtrStdConsole();
ctr_object** get_CtrStdNil();
ctr_object** get_CtrStdGC();
ctr_object** get_CtrStdMap();
ctr_object** get_CtrStdHashMap();
ctr_object** get_CtrStdArray();
ctr_object** get_CtrStdIter();
ctr_object** get_CtrStdFile();
ctr_object** get_CtrStdSystem();
ctr_object** get_CtrStdDice();
ctr_object** get_CtrStdCommand();
ctr_object** get_CtrStdSlurp();
ctr_object** get_CtrStdShell();
ctr_object** get_CtrStdClock();
ctr_object** get_CtrStdFlow();
ctr_object** get_CtrExceptionType();
ctr_object** get_CtrStdBreak();
ctr_object** get_CtrStdContinue();
ctr_object** get_CtrStdExit();
ctr_object** get_CtrStdReflect();
ctr_object** get_CtrStdReflect_cons();
ctr_object** get_CtrStdFiber();
ctr_object** get_CtrStdThread();
ctr_object** get_CtrStdSymbol();
ctr_object** get_ctr_first_object();
ctr_object** get_CTR_FILE_STDIN();
ctr_object** get_CTR_FILE_STDOUT();
ctr_object** get_CTR_FILE_STDERR();
ctr_object** get_CTR_FILE_STDIN_STR();
ctr_object** get_CTR_FILE_STDOUT_STR();
ctr_object** get_CTR_FILE_STDERR_STR();

ctr_object* ctr_exception_getinfo(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_internal_ex_data();

ctr_object* ctr_internal_find_overload(ctr_object*,ctr_argument*);
/**
* @internal
 * standard instrumentor, do not override.
 * only combine
 */
CTR_H_DECLSPEC ctr_object* ctr_instrumentor_funcs;
CTR_H_DECLSPEC ctr_object* ctr_global_instrum;
CTR_H_DECLSPEC int ctr_instrument;

CTR_H_DECLSPEC int ctr_internal_next_return;

/**
 * Hashkey
 */
CTR_H_DECLSPEC char CtrHashKey[16];

/**
 * CLI Arguments
 */
CTR_H_DECLSPEC int ctr_argc;
CTR_H_DECLSPEC char** ctr_argv;

/**
 * Mode of Operation
 */
CTR_H_DECLSPEC char* ctr_mode_input_file;
CTR_H_DECLSPEC char* ctr_mode_interactive;


/**
 * Lexer functions
 */
struct lexer_state {
	int string_interpolation;
	int ivarlen;
	ctr_size ctr_clex_tokvlen;
	char *ctr_clex_buffer;
	char *ctr_code;
	char *ctr_code_st;
	char *ctr_code_eoi;
	char *ctr_eofcode;
	char *ctr_clex_oldptr;
	char *ctr_clex_olderptr;
	int ctr_clex_verbatim_mode;
	uintptr_t ctr_clex_verbatim_mode_insert_quote;
	int ctr_clex_old_line_number;
	int ctr_transform_lambda_shorthand;
	char *ivarname;
    int inject_index; //<- this is not sure to work
};
void 	ctr_clex_load(char* prg);
int 	ctr_clex_tok();
char* 	ctr_clex_tok_value();
long    ctr_clex_tok_value_length();
void 	ctr_clex_putback();
char*	ctr_clex_readstr();
char*	ctr_clex_readfstr();
char* ctr_clex_scan(char c);
char* ctr_clex_scan_balanced(char c, char d);
char*   ctr_clex_tok_describe( int token );
int     ctr_clex_save_state();
int     ctr_clex_dump_state(struct lexer_state*);
int     ctr_clex_restore_state( int id );
int     ctr_clex_load_state(struct lexer_state);
int     ctr_clex_inject_token( int token, const char* value, const int vlen, const int real_vlen);
void    ctr_match_toggle_pragma();
void    ctr_lex_parse_pragma();
CTR_H_DECLSPEC char*   ctr_clex_code_init;
CTR_H_DECLSPEC char*   ctr_clex_code_end;
CTR_H_DECLSPEC char*   ctr_clex_oldptr;
CTR_H_DECLSPEC char*   ctr_clex_olderptr;
CTR_H_DECLSPEC int     ctr_clex_quiet;
CTR_H_DECLSPEC char*   ctr_clex_keyword_var;
CTR_H_DECLSPEC char*   ctr_clex_keyword_me;
CTR_H_DECLSPEC char*   ctr_clex_keyword_my;
CTR_H_DECLSPEC char*   ctr_clex_keyword_const;
CTR_H_DECLSPEC char*   ctr_clex_keyword_static;
CTR_H_DECLSPEC int     ctr_clex_line_number;
CTR_H_DECLSPEC int     ctr_clex_old_line_number;

CTR_H_DECLSPEC ctr_size ctr_clex_keyword_const_len;
CTR_H_DECLSPEC ctr_size ctr_clex_keyword_my_len;
CTR_H_DECLSPEC ctr_size ctr_clex_keyword_var_len;
CTR_H_DECLSPEC ctr_size ctr_clex_keyword_static_len;

/**
 * Lexer properties
 */
CTR_H_DECLSPEC ctr_size ctr_clex_len;
CTR_H_DECLSPEC ctr_size ctr_program_length;
CTR_H_DECLSPEC int ctr_clex_line_number;
CTR_H_DECLSPEC int ctr_clex_char_num;
CTR_H_DECLSPEC int ctr_transform_lambda_shorthand;

unsigned long ctr_lex_position();
void ctr_lex_load(char* prg, size_t len);
char* ctr_lex_tok_value();
long ctr_lex_tok_value_length();
void ctr_lex_putback();
int ctr_lex_tok();
char* ctr_lex_readstr();
void ctr_lex_skip(int len);
ctr_object* ctr_lex_get_buf_str();

// Pragmas
CTR_H_DECLSPEC ctr_code_pragma* oneLineExpressions;
CTR_H_DECLSPEC ctr_code_pragma* flexibleConstructs;
CTR_H_DECLSPEC ctr_code_pragma* regexLineCheck;
CTR_H_DECLSPEC ctr_code_pragma* callShorthand;
CTR_H_DECLSPEC ctr_code_pragma* nextCallLazy;
CTR_H_DECLSPEC ctr_code_pragma* extensionsPra;

// XFrozen
#define CTR_EXT_FROZEN_K 1
// XPureLambda
// force enables XFrozen
#define CTR_EXT_PURE_FS  (1|2)
// XNakedAsmBlock
#define CTR_EXT_ASM_BLOCK 4

extern int ctr_did_side_effect;
void ctr_mksrands(char* buf);

/**
 * UTF-8 functions
 */
ctr_size getBytesUtf8(char* strval, long startByte, ctr_size lenUChar);
ctr_size ctr_getutf8len(char* strval, ctr_size max);
ctr_size ctr_getutf8clustercount(char* strval, ctr_size max);
int ctr_utf8_is_one_cluster(char* strval, ctr_size max);
int ctr_utf8size(char c);

/**
 * Parser functions
 */
ctr_tnode* ctr_cparse_parse(char* prg, char* pathString);
ctr_tnode* ctr_cparse_expr(int mode);
ctr_tnode* ctr_cparse_ret();
CTR_H_DECLSPEC int     ctr_cparse_quiet;
ctr_object* ctr_ast_from_node (ctr_tnode * node);
int ctr_ast_is_splice (ctr_object* obj);
ctr_object* ctr_ast_splice (ctr_object* obj);

#define CTR_CONTEXT_VECTOR_DEPTH  50000

CTR_H_DECLSPEC char* ctr_last_parser_error;
CTR_H_DECLSPEC int        ctr_cparse_calltime_name_id;
CTR_H_DECLSPEC ctr_tnode* ctr_cparse_calltime_names[CTR_CONTEXT_VECTOR_DEPTH];
/**
 * Abstract Tree Walker functions
 */
CTR_H_DECLSPEC uint64_t    ctr_cwlk_subprogram;
ctr_object* ctr_cwlk_run(ctr_tnode* program);
ctr_object* ctr_cwlk_expr(ctr_tnode* node, char* wasReturn);
ctr_tnode* ctr_unmarshal_ast(char* stream, size_t avail, size_t* consumed);
void ctr_marshal_ast(ctr_tnode* rnode, char** stream, size_t* stream_len, size_t* used);
/**
 * Internal World functions
 */
ctr_object* ctr_get_last_trace(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_get_last_trace_stringified(ctr_object* myself, ctr_argument* argumentList);
void        ctr_initialize_world();
void        ctr_initialize_world_minimal();
void 		ctr_initialize_ex();
char*       ctr_internal_memmem(char* haystack, long hlen, char* needle, long nlen, int reverse );
void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
void        ctr_internal_object_add_property_with_hash(ctr_object* owner, ctr_object* key, uint64_t hash, ctr_object* value, int m);
void        ctr_internal_object_set_property(ctr_object* owner, ctr_object* key, ctr_object* value, int is_method);
void        ctr_internal_object_delete_property(ctr_object* owner, ctr_object* key, int is_method);
void        ctr_internal_object_delete_property_with_hash(ctr_object* owner, ctr_object* key, uint64_t hash, int is_method);
ctr_object* ctr_internal_object_find_property(ctr_object* owner, ctr_object* key, int is_method);
ctr_object* ctr_internal_object_find_property_with_hash(ctr_object* owner, ctr_object* key, uint64_t hash, int is_method);
ctr_object* ctr_internal_object_find_property_or_create_with_hash (ctr_object * owner, ctr_object * key, uint64_t hashKey, int is_method);
ctr_object* ctr_internal_object_find_property_ignore(ctr_object* owner, ctr_object* key, int is_method, int ignore);
uint64_t    ctr_internal_index_hash(ctr_object* key);
uint64_t    ctr_internal_alt_hash(ctr_object* key);
void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
ctr_object* ctr_internal_cast2bool( ctr_object* o );
ctr_object* ctr_internal_cast2number(ctr_object* o);
ctr_object* ctr_internal_cast2string( ctr_object* o );
ctr_object* ctr_internal_create_object(int type);
ctr_object* ctr_internal_create_mapped_object(int type, int shared);
ctr_object *ctr_internal_create_standalone_object (int type);
void ctr_transfer_object_ownership(ctr_object* to, ctr_object* what);
inline ctr_object *ctr_internal_create_mapped_standalone_object (int type, int shared);
void ctr_internal_delete_standalone_object (ctr_object* o);
void*       ctr_internal_plugin_find( ctr_object* key );
void*       ctr_internal_plugin_find_base( char const* key );
ctr_object* ctr_format_str(const char* str_format, ...);
ctr_object* ctr_find_(ctr_object* key, int noerror);
ctr_object* ctr_find(ctr_object* key);
ctr_object* ctr_find_in_my(ctr_object* key);
ctr_object* ctr_assign_value(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_local_by_ref(ctr_object* key, ctr_object* val);
ctr_object* ctr_hand_value_to_global(ctr_object* key, ctr_object* val);
ctr_object* ctr_const_assign_value(ctr_object* key, ctr_object* o, ctr_object* context);
char*       ctr_internal_readf(char* file_name, uint64_t* size_allocated);
char*       ctr_internal_readfp(FILE* fp, uint64_t* size_allocated);
void        ctr_internal_debug_tree(ctr_tnode* ti, int indent);
void 		ctr_capture_refs(ctr_tnode* ti, ctr_object* block);
ctr_object* ctr_get_responder(ctr_object* receiverObject, char* message, long vlen);
ctr_object* ctr_send_message(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
ctr_object* ctr_send_message_blocking(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
ctr_object* ctr_send_message_async(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
ctr_object* ctr_send_message_variadic(ctr_object* myself, char* message, int msglen, int count, ...) ;
ctr_object* ctr_invoke_variadic(ctr_object* myself, ctr_object * (*fun)(ctr_object *, ctr_argument *), int count, ...);
ctr_argument* ctr_allocate_argumentList (int count, ...);
void ctr_free_argumentList (ctr_argument* argumentList);
void ctr_internal_create_func(ctr_object* o, ctr_object* key, ctr_object* (*func)( ctr_object*, ctr_argument* ) );
int ctr_is_primitive(ctr_object* object);
ctr_object* ctr_get_stack_trace();
void ctr_print_stack_trace();
ctr_object* ctr_get_or_create_symbol_table_entry  (const char* name, ctr_size len);
ctr_object* ctr_get_or_create_symbol_table_entry_s(const char* name, ctr_size len);
fixity_lookup_rv ctr_lookup_fix(const char* name, int length);
void ctr_set_fix(const char* name, int length, int fix, int prec, int lazy);
void clear_fixity_map();
int ctr_str_count_substr(char *str, char *substr, int overlap);

#define CTR_TRACE_IGNORE_VEC_DEPTH 1024
#define CTR_TRACE_IGNORE_LENGTH 2048
CTR_H_DECLSPEC char ignore_in_trace[CTR_TRACE_IGNORE_LENGTH][CTR_TRACE_IGNORE_VEC_DEPTH]; //ignore named files.
int trace_ignore_count;
extern char* SystemTZ;
/**
 * Scoping functions
 */
void ctr_open_context();
void ctr_close_context();
void ctr_switch_context();

/**
 * Global Scoping variables
 */
struct ctr_context_t {
    ctr_object* contexts[CTR_CONTEXT_VECTOR_DEPTH];
    int id;
};
CTR_H_DECLSPEC void ctr_dump_context(struct ctr_context_t*);
CTR_H_DECLSPEC void ctr_load_context(struct ctr_context_t);
#ifndef CTR_GLOBALS_DEFINE
__thread extern ctr_tnode* ctr_callstack[CTR_CONTEXT_VECTOR_DEPTH]; //That should be enough... right?
__thread extern uint8_t ctr_callstack_index;
__thread extern CTR_H_DECLSPEC int ctr_context_id;
// __thread extern ctr_object* ctr_contexts[CTR_CONTEXT_VECTOR_DEPTH];
#else
__thread CTR_H_DECLSPEC ctr_tnode* ctr_callstack[CTR_CONTEXT_VECTOR_DEPTH]; //That should be enough... right?
__thread CTR_H_DECLSPEC uint8_t ctr_callstack_index;
__thread CTR_H_DECLSPEC int ctr_context_id;
// __thread CTR_H_DECLSPEC ctr_object* ctr_contexts[CTR_CONTEXT_VECTOR_DEPTH];
#endif
CTR_H_DECLSPEC ctr_object* ctr_contexts[CTR_CONTEXT_VECTOR_DEPTH];

/**
 * Nil Interface
 */
ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_nil_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_nil_to_number(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_nil_to_boolean(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_nil_assign(ctr_object* myself, ctr_argument* argumentList);

/**
 * Object Interface
 */
ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_ctor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_assign(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_hash(ctr_object* myself, ctr_argument* argumentList);
// ctr_object* ctr_object_ahash(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_attr_accessor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_attr_reader(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_attr_writer(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_make_hiding(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_swap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_id(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_done(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_message(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_elvis_op(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_if_false(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_if_true(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_if_tf(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_learn_meaning(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_object_to_string(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_object_to_number(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_object_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_object_remote(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_object_respond_and(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_object_respond_and_and(ctr_object* myself, ctr_argument* ctr_argumentList);

/**
 * Boolean Interface
 */
ctr_object* ctr_bool_if_true(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_if_false(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_if_tf(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_xor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_flip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_assign(ctr_object* myself, ctr_argument* argumentList);

/**
 * Number Interface
 */
ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_factorial(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_exp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_shr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_shl(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_xor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_or(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_and(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_uint_binrep(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_string_base(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_step_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_byte(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_qualify(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_respond_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_assign(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_number_negate(ctr_object * myself, ctr_argument * argumentList);

/**
 * String Interface
 */
ctr_object* ctr_string_escape_ascii(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_empty(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_multiply(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_ltrim(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_rtrim(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_padding_left(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_padding_right(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_html_escape(ctr_object* myself, ctr_argument* argumentList);
// ctr_object* ctr_string_literal_escape(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_count_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_slice(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_put_at(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_fmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_imap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_filter(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_re_index_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_starts_with(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_ends_with(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_re_last_index_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_replace_with(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_split_re(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_split_re_gen(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_lower1st(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_upper1st(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_find_pattern_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_find_pattern_options_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_reg_replace(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_reg_compile(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_is_regex_pcre(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_contains_pattern(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_contains(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_hash_with_key(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_string( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_format( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_format_map( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_eval( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_exec( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_quotes_escape( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_dquotes_escape( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_characters( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_to_byte_array( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_append_byte(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_csub( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_cadd( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_randomize_bytes(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_string_reverse(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_is_ctor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_assign(ctr_object* myself, ctr_argument* argumentList);

/**
 * Block Interface
 */
ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_specialise(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_runall(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_run_variadic(ctr_object* myself, int count, ...);
ctr_object* ctr_block_run_variadic_my(ctr_object* myself, ctr_object* my, int count, ...);
ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_catch_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_while_false(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_forever(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my);
ctr_object* ctr_block_run_here(ctr_object* myself, ctr_argument* argList, ctr_object* my);
ctr_object* ctr_block_times(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_assign(ctr_object* myself, ctr_argument* argumentList);

/**
 * Array Interface
 */
ctr_object* ctr_array_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_copy(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_new_and_push(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_fmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_imap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_fmap_inp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_imap_inp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_foldl(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_foldl0(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_filter(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_filter_v(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_select_from_if(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_push_imm(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_reverse(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_shift(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_empty(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_index(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_contains(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_from_length(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_skip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_zip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_internal_product(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_zip_with(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_head(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_tail(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_init(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_last(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_map_v(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_min(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_max(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_sum(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_product(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_multiply(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_fill(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_column(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_assign(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_intersperse(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_slice(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_chunks(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_every_do_fill(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_array_all(ctr_object * myself, ctr_argument * argumentList);
ctr_object *ctr_array_any(ctr_object * myself, ctr_argument * argumentList);
ctr_object* ctr_build_immutable(ctr_tnode* node);

/**
 * HashMap Interface
 */
ctr_object* ctr_hmap_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_merge(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_keys(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_values(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_put(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_rm(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_get(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_get_or_insert(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_empty(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_each(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_fmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_fmap_inp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_kvmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_kvlist(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_contains(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_flip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_assign(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_hmap_to_string(ctr_object* myself, ctr_argument* argumentList);

/**
 * Generic Map Interface
 */
ctr_object* ctr_map_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_new_(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_merge(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_keys(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_values(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_rm(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_get_or_insert(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_empty(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_fmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_fmap_inp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_kvmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_kvlist(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_contains(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_flip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_assign(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_to_string(ctr_object* myself, ctr_argument* argumentList);

/**
 * Console Interface
 */
ctr_object* ctr_console_writeln(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_red(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_green(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_yellow(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_blue(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_magenta(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_cyan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_reset(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_tab(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_line(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_clear(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_clear_line(ctr_object* myself, ctr_argument* argumentList);

/**
 * File Interface
 */
ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_special(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_get_descriptor(ctr_object * myself, ctr_argument * argumentList);
ctr_object* ctr_file_ddup(ctr_object* myself, ctr_argument * argumentList);
ctr_object* ctr_file_memopen (ctr_object * myself, ctr_argument * argumentList);
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_rpath(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_expand(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_generate_lines(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_include_here(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_close(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_flush(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_read_bytes(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_write_bytes(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_seek(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_seek_rewind(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_seek_end(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_descriptor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_lock_generic(ctr_object* myself, ctr_argument* argumentList, int lock);
ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_unlock(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_list(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_tmp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_stdext_path(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_assign(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_mkdir(ctr_object* myself, ctr_argument* argumentList);
/**
 * Command Object Interface
 */
void ctr_int_handler(int signal);
CTR_H_DECLSPEC ctr_object* ctr_signal_map;
ctr_object* ctr_command_argument(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_num_of_args(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_waitforinput(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_getc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_input(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_set_INT_handler(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_get_env(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_set_env(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_exit(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_flush(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_forbid_shell(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_forbid_file_write(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_forbid_file_read(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_forbid_include(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_forbid_fork(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_countdown(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_fork(ctr_object* myself, ctr_argument* ctr_argumentList);
ctr_object* ctr_command_message(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_listen(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_join(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_log(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_warn(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_err(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_crit(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_pid(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_sig(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_sigmap(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_accept(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_accepti4(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_accept_number(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_remote(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_default_port(ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_command_chdir(ctr_object* myself, ctr_argument* argumentList);


void ctr_check_permission( uint8_t operationID );
int  ctr_check_permission_internal( uint8_t operationID );
CTR_H_DECLSPEC uint8_t ctr_command_security_profile;
CTR_H_DECLSPEC uint64_t ctr_command_tick;
CTR_H_DECLSPEC uint64_t ctr_command_maxtick;
CTR_H_DECLSPEC ctr_object* (*ctr_secpro_eval_whitelist[64])(ctr_object*, ctr_argument*);
CTR_H_DECLSPEC ctr_object* ctr_global_interrupt_handler;

/**
 * Clock Interface
 */
ctr_object* ctr_clock_change( ctr_object* myself, ctr_argument* argumentList, uint8_t forward );
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_time(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_new_set(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_like(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_day(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_month(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_year(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_hour(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_minute(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_second(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_weekday(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_yearday(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_week(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_set_day(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_set_month(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_set_year(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_set_hour(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_set_minute(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_set_second(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_get_time( ctr_object* myself, ctr_argument* argumentList, char part );
ctr_object* ctr_clock_set_time( ctr_object* myself, ctr_argument* argumentList, char part );
ctr_object* ctr_clock_set_zone( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_get_zone( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_isdst( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_format( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_add( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_subtract( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_processor_time( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_processor_ticks_ps( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_time_exec( ctr_object* myself, ctr_argument* argumentList );
ctr_object* ctr_clock_time_exec_s( ctr_object* myself, ctr_argument* argumentList );
void ctr_clock_init( ctr_object* clock );

/**
 * Slurp Interface
 */
ctr_object* ctr_slurp_obtain(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_slurp_respond_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_slurp_respond_to_and(ctr_object* myself, ctr_argument* argumentList);

/**
 * Shell Interface
 */
ctr_object* ctr_shell_call(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_shell_open(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_shell_respond_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_shell_respond_to_and(ctr_object* myself, ctr_argument* argumentList);


/**
 * Garbage Collector Object Interface
 */
CTR_H_DECLSPEC int CTR_LIMIT_MEM;
ctr_object* ctr_gc_collect(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_with_gc_disabled(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_dust(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_object_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_kept_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_kept_alloc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_sticky_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_setmode(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_setautoalloc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_setmemlimit(ctr_object* myself, ctr_argument* argumentList);
void ctr_gc_sweep( int all );
void ctr_gc_pin(void* alloc_ptr);
void ctr_gc_unpin(void* alloc_ptr);

/**
 * Language Reflection Interface
 */
static ctr_object* ctr_reflect_map_type_descriptor __attribute__((unused));

ctr_object* ctr_reflect_add_glob(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_add_local(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_add_my(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_dump_context_prop(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_dump_context_spec_prop(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_dump_context_spec(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_dump_context(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_this_context(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_find_obj(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_find_obj_ex(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_set_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_cb_ac(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_cb_add_param(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_fn_copy(ctr_object* myself, ctr_argument* argumentList);
ctr_tnode* ctr_reflect_internal_term_rewrite(ctr_tnode* pfn, int nodety, ctr_tnode* rewrite_term, int replace_mask);
ctr_object* ctr_reflect_share_memory(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_link_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_child_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_is_linked_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_generate_inheritance_tree(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_get_primitive_link(ctr_object* object);
ctr_object* ctr_reflect_describe_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_describe_value(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_type_descriptor_print(ctr_object* myself, ctr_argument* argumentList);
int ctr_reflect_check_bind_valid(ctr_object* from, ctr_object* to, int err);
ctr_object* ctr_reflect_check_bind_valid_v(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_bind(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_cons_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_try_serialize_block(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_cons_value(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_cons_str(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_get_first_link(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_get_responder(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_for_object(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_for_object_ctx(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_closure_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_get_primitive(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_get_property(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_set_property(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_has_own_responder(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_object_delegate_get_responder(ctr_object* itself, ctr_argument* argumentList);
ctr_object* ctr_reflect_object_get_responder(ctr_object* itself, ctr_argument* argumentList);
ctr_object* ctr_reflect_rawmsg(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_instrmsg(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_register_instrumentor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_unregister_instrumentor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_get_instrumentor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_ginstr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_noginstr(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_world_snap(ctr_object*, ctr_argument*);
ctr_object* ctr_reflect_run_glob(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_for_object_in_ctx (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_for_object_inside_ctx (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_for_object_in_ctx_as_world (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_for_object_in_ctx_as_main (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_run_in_new_ctx(ctr_object * myself, ctr_argument * argumentList);
ctr_object* ctr_reflect_compilerinfo (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_delegate_set_private_property(ctr_object* itself, ctr_argument* argumentList);
ctr_object* ctr_reflect_set_ignore_file (ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_nat_type (ctr_object* myself, ctr_argument* argumentList);
ctr_argument* ctr_array_to_argument_list (ctr_object * arr, ctr_argument * provided);
int ctr_internal_has_own_responder(ctr_object* myself, ctr_object* meth);
int ctr_internal_has_responder(ctr_object* myself, ctr_object* meth);
/* Defined in marshal.c v*/
ctr_object* ctr_reflect_marshal(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_reflect_unmarshal(ctr_object* myself, ctr_argument* argumentList);
/* Defined in marshal.c ^*/
// ctr_object* ctr_reflect_dump_function(ctr_object* myself, ctr_argument* argumentList);

/**
 * Fiber Co-Processing Interface
 */
ctr_object* ctr_fiber_spawn(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_fiber_yield(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_fiber_join_all(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_fiber_tostring(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_fiber_yielded(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_fiber_assign(ctr_object* myself, ctr_argument* argumentList);
void ctr_fiber_begin_init(void);

ctr_object* ctr_thread_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_make_set_target(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_set_target(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_run(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_finished(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_detach(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_join(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_id(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_name(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_names(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_thread_to_string(ctr_object* myself, ctr_argument* argumentList);

/** ImportLib **/
CTR_H_DECLSPEC ctr_object* CtrStdImportLib;
CTR_H_DECLSPEC ctr_object* CtrStdImportLib_SearchPaths;
ctr_object* ctr_importlib_begin(ctr_object* myself, ctr_argument* argumentList);

/** Inject **/
CTR_H_DECLSPEC ctr_object* CtrStdInject;
voidptrfn_t ctr_inject_compiled_state_release_hook;
ctr_object* ctr_inject_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_inject_defined_functions(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_inject_compile(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_get_symbol(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_run(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_run_named(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_add_inclp(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_get_inclp(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_add_libp(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_get_libp(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_export_f(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_add_lib(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_set_error_handler(ctr_object* myself, ctr_argument* argumentList);
ctr_object *ctr_inject_generate_output(ctr_object *myself, ctr_argument *argumentList);
ctr_object *ctr_inject_finish(ctr_object *myself, ctr_argument *argumentList);

/** FFI **/
#include "ctypes.h"
void ctr_ffi_begin();

/**
 * Global Garbage Collector variables
 */
CTR_H_DECLSPEC int ctr_gc_dust_counter;
CTR_H_DECLSPEC int ctr_gc_object_counter;
CTR_H_DECLSPEC int ctr_gc_kept_counter;
CTR_H_DECLSPEC int ctr_gc_sticky_counter;
CTR_H_DECLSPEC int ctr_gc_mode;

CTR_H_DECLSPEC uint64_t ctr_gc_alloc;
CTR_H_DECLSPEC uint64_t ctr_gc_memlimit;

/**
 * Misc Interfaces
 */
ctr_object* ctr_dice_throw(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_dice_sides(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_dice_rand(ctr_object* myself, ctr_argument* argumentList);

/**
 * Literal Constructors (internal only)
 */
ctr_object* ctr_build_empty_string();
ctr_object* ctr_build_string(char* object, long vlen);
ctr_object* ctr_build_string_from_cformat(char* format, int count, ...);
ctr_object* ctr_build_block(ctr_tnode* node);
ctr_object* ctr_build_listcomp(ctr_tnode* node);
ctr_object* ctr_build_number(char* object);
ctr_object* ctr_build_number_from_string(char* fixedStr, ctr_size strLength);
ctr_object* ctr_build_number_from_float(ctr_number floatNumber);
ctr_object* ctr_build_bool(int truth);
ctr_object* ctr_build_nil();
ctr_object* ctr_build_string_from_cstring( char* str );
void ctr_gc_internal_collect();
ctr_object* ctr_gc_sweep_this( ctr_object* myself, ctr_argument* argumentList );


void* ctr_heap_allocate_shared( size_t size );
void* ctr_heap_allocate( size_t size );
#ifdef withBoehmGC_P
void* ctr_heap_allocate_typed( size_t size, int type );
#endif
void* ctr_heap_allocate_tracked( size_t size );
void  ctr_heap_free( void* ptr );
void  ctr_heap_free_shared( void* ptr );
void  ctr_heap_free_rest();
void* ctr_heap_reallocate(void* oldptr, size_t size );
size_t ctr_heap_get_latest_tracking_id();
void* ctr_heap_reallocate_tracked(size_t tracking_id, size_t size );
char* ctr_heap_allocate_cstring( ctr_object* o );
char* ctr_heap_allocate_cstring_shared( ctr_object* o );

void initiailize_base_extensions();

CTR_H_DECLSPEC uint8_t  ctr_accept_n_connections;
CTR_H_DECLSPEC uint16_t ctr_default_port;

#include "citron_ensure.h"
#include "citron_conv.h"

CTR_H_DECLSPEC ctr_string CTR_CLEX_KW_ME_SV, CTR_CLEX_KW_THIS_SV, CTR_CLEX_US_SV, CTR_CLEX_KW_RESPONDTO_SV;
CTR_H_DECLSPEC ctr_object CTR_CLEX_KW_ME,    CTR_CLEX_KW_THIS,    CTR_CLEX_US,    CTR_CLEX_KW_RESPONDTO;
CTR_H_DECLSPEC int ctr_current_node_is_return;

static inline void ctr_linkstr();
void ctr_set_link_all(ctr_object*, ctr_object*);
void ctr_deallocate_argument_list(ctr_argument*);
int ctr_internal_object_is_equal(ctr_object*, ctr_object*);
int ctr_internal_object_is_constructible_(ctr_object*, ctr_object*, int);

static ctr_object* ctr_world_ptr;

#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */
	#define __COMPILER__NAME__OP "Clang/LLVM " __clang_version__
#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */
	#define __COMPILER__NAME__OP "Intel CC"
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
	#define __COMPILER__NAME__OP "GCC/G++ " __VERSION__
#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/aC++. ---------------------------------- */
	#define __COMPILER__NAME__OP "HP aCC"
#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */
	#define __COMPILER__NAME__OP "IBM XL"
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
	#define __COMPILER__NAME__OP "MSC " _MSC_VER
#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */
	#define __COMPILER__NAME__OP "Some Fucked up shit"
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */
	#define __COMPILER__NAME__OP "Solaris Studio"
#endif

#ifdef  __cplusplus
}
#endif

#endif //CTR_H_GUARD
