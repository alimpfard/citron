#include "citron.h"
#include "coroutine.h"
#include <stdio.h>

/**
 * [Array|String] letEqual: [Array|Object] in: [Block]
 *
 * Assigns local values to the block, and runs it as a closure
 */
ctr_object *ctr_block_let(ctr_object * myself, ctr_argument * argumentList)
{
	ctr_object *defs = argumentList->object, *block =
	    argumentList->next->object;
	ctr_object *result;
	ctr_open_context();
	if (myself->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		for (ctr_size i = 0;
		     i < myself->value.avalue->head - myself->value.avalue->tail
		     && (defs->info.type == CTR_OBJECT_TYPE_OTARRAY
			 && i <
			 defs->value.avalue->head - defs->value.avalue->tail);
		     i++) {
			switch (defs->info.type) {
			case CTR_OBJECT_TYPE_OTARRAY:
				ctr_assign_value_to_local(myself->value.avalue->
							  elements[i],
							  defs->value.avalue->
							  elements[i]);
				break;
			default:
				ctr_assign_value_to_local(myself->value.avalue->
							  elements[i], defs);
				break;
			}
		}
		result = ctr_block_run_here(block, NULL, block);
	} else {
		myself = ctr_internal_cast2string(myself);
		ctr_assign_value_to_local(myself, defs);
		result = ctr_block_run_here(block, NULL, block);
	}
	ctr_close_context();
	return result;
}

ctr_object *CtrStdAst;
/**
 * [AST] parse: [String]
 *
 * parse string into citron ast
 */
ctr_object *ctr_ast_parse(ctr_object * myself, ctr_argument * argumentList)
{
	ctr_object *psec = argumentList->object;
	char *prg = ctr_heap_allocate_cstring(psec);
	int last_q = ctr_cparse_quiet;
	ctr_cparse_quiet = 1;
	ctr_program_length = psec->value.svalue->vlen + 1;
	ctr_tnode *ped = ctr_cparse_parse(prg, "ASTparse");
	ctr_cparse_quiet = last_q;
	ctr_object *ast = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	ast->link = CtrStdAst;
	ast->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
	ast->value.rvalue->ptr = ped;
	ast->value.rvalue->type = CTR_AST_TYPE;
	return ast;
}

ctr_object *ctr_ast_instrcount(ctr_object * myself, ctr_argument * argumentList)
{
	// if (myself == CtrStdAst) return ctr_build_number_from_float(0);
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	ctr_tnode *node = myself->value.rvalue->ptr;
	int count = -1;
	ctr_tlistitem *pitem = node->nodes;
	while (pitem) {
		count++;
		pitem = pitem->next;
	}
	return ctr_build_number_from_float(count);
}

ctr_object *ctr_ast_nth(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
 err:		;
		CtrStdFlow =
		    ctr_build_string_from_cstring
		    ("Null ast node or count out of range");
		return CtrStdNil;
	}
	int n = argumentList->object->value.nvalue;
	ctr_tnode *node = myself->value.rvalue->ptr;
	ctr_tlistitem *pitem = node->nodes;
	while (n) {
		if (!pitem)
			goto err;
		n--;
		pitem = pitem->next;
	}
	if (!pitem)
		goto err;
	ctr_object *ast = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	ast->link = CtrStdAst;
	ast->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
	ast->value.rvalue->ptr = pitem->node;
	ast->value.rvalue->type = CTR_AST_TYPE;
	return ast;
}

static char const *ctr_ast_tystr(ctr_tnode * ast)
{
	if (!ast)
		return "(nil)";
	switch (ast->type) {
	case CTR_AST_NODE_EXPRASSIGNMENT:
		return "EXPRASSIGNMENT";
	case CTR_AST_NODE_EXPRMESSAGE:
		return "EXPRMESSAGE";
	case CTR_AST_NODE_UNAMESSAGE:
		return "UNAMESSAGE";
	case CTR_AST_NODE_BINMESSAGE:
		return "BINMESSAGE";
	case CTR_AST_NODE_KWMESSAGE:
		return "KWMESSAGE";
	case CTR_AST_NODE_LTRSTRING:
		return "LTRSTRING";
	case CTR_AST_NODE_REFERENCE:
		return "REFERENCE";
	case CTR_AST_NODE_LTRNUM:
		return "LTRNUM";
	case CTR_AST_NODE_CODEBLOCK:
		return "CODEBLOCK";
	case CTR_AST_NODE_RETURNFROMBLOCK:
		return "RETURNFROMBLOCK";
	case CTR_AST_NODE_IMMUTABLE:
		return "IMMUTABLE";
	case CTR_AST_NODE_PARAMLIST:
		return "PARAMLIST";
	case CTR_AST_NODE_INSTRLIST:
		return "INSTRLIST";
	case CTR_AST_NODE_ENDOFPROGRAM:
		return "ENDOFPROGRAM";
	case CTR_AST_NODE_NESTED:
		return "NESTED";
	case CTR_AST_NODE_LTRBOOLTRUE:
		return "LTRBOOLTRUE";
	case CTR_AST_NODE_LTRBOOLFALSE:
		return "LTRBOOLFALSE";
	case CTR_AST_NODE_LTRNIL:
		return "LTRNIL";
	case CTR_AST_NODE_PROGRAM:
		return "PROGRAM";
	default:
		return "UNKNOWN";
	}
}

static char const *ctr_ast_modifier_str(char modifier)
{
	switch (modifier) {
	case 1:
		return "My";
	case 2:
		return "Var";
	case 3:
		return "Const";
	default:
		return "None";
	}
}

static int ctr_ast_tyfstr(char *type)
{
	if (strcasecmp("EXPRASSIGNMENT", type) == 0)
		return CTR_AST_NODE_EXPRASSIGNMENT;
	if (strcasecmp("EXPRMESSAGE", type) == 0)
		return CTR_AST_NODE_EXPRMESSAGE;
	if (strcasecmp("UNAMESSAGE", type) == 0)
		return CTR_AST_NODE_UNAMESSAGE;
	if (strcasecmp("BINMESSAGE", type) == 0)
		return CTR_AST_NODE_BINMESSAGE;
	if (strcasecmp("KWMESSAGE", type) == 0)
		return CTR_AST_NODE_KWMESSAGE;
	if (strcasecmp("LTRSTRING", type) == 0)
		return CTR_AST_NODE_LTRSTRING;
	if (strcasecmp("REFERENCE", type) == 0)
		return CTR_AST_NODE_REFERENCE;
	if (strcasecmp("LTRNUM", type) == 0)
		return CTR_AST_NODE_LTRNUM;
	if (strcasecmp("CODEBLOCK", type) == 0)
		return CTR_AST_NODE_CODEBLOCK;
	if (strcasecmp("RETURNFROMBLOCK", type) == 0)
		return CTR_AST_NODE_RETURNFROMBLOCK;
	if (strcasecmp("IMMUTABLE", type) == 0)
		return CTR_AST_NODE_IMMUTABLE;
	if (strcasecmp("PARAMLIST", type) == 0)
		return CTR_AST_NODE_PARAMLIST;
	if (strcasecmp("INSTRLIST", type) == 0)
		return CTR_AST_NODE_INSTRLIST;
	if (strcasecmp("ENDOFPROGRAM", type) == 0)
		return CTR_AST_NODE_ENDOFPROGRAM;
	if (strcasecmp("NESTED", type) == 0)
		return CTR_AST_NODE_NESTED;
	if (strcasecmp("LTRBOOLTRUE", type) == 0)
		return CTR_AST_NODE_LTRBOOLTRUE;
	if (strcasecmp("LTRBOOLFALSE", type) == 0)
		return CTR_AST_NODE_LTRBOOLFALSE;
	if (strcasecmp("LTRNIL", type) == 0)
		return CTR_AST_NODE_LTRNIL;
	if (strcasecmp("PROGRAM", type) == 0)
		return CTR_AST_NODE_PROGRAM;
	return CTR_AST_NODE_ENDOFPROGRAM;
}

static char ctr_ast_modifier_fstr(char *mod)
{
	if (strcasecmp("My", mod))
		return 1;
	if (strcasecmp("Var", mod))
		return 2;
	if (strcasecmp("Const", mod))
		return 3;
	return 0;
}

ctr_object *ctr_ast_tostring(ctr_object * myself, ctr_argument * argumentList)
{
	char buf[1024];
	ctr_tnode *ptr =
	    myself->value.rvalue ? myself->value.rvalue->ptr : NULL;
	(void)sprintf(buf, "<AST:%s<%.*s> at %p>", ctr_ast_tystr(ptr),
		      (int)(ptr ? ptr->vlen : 5), ptr ? ptr->value : "(nil)",
		      ptr);
	return ctr_build_string_from_cstring(buf);
}

ctr_object *ctr_ast_get_value(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	ctr_tnode *node = myself->value.rvalue->ptr;
	if (!node->value)
		return ctr_build_empty_string();
	return ctr_build_string(node->value, node->vlen);
}

ctr_object *ctr_ast_get_type(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	ctr_tnode *node = myself->value.rvalue->ptr;
	return ctr_build_string_from_cstring((char *)ctr_ast_tystr(node));
}

ctr_object *ctr_ast_get_mod(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	ctr_tnode *node = myself->value.rvalue->ptr;
	return ctr_build_string_from_cstring((char *)
					     ctr_ast_modifier_str(node->
								  modifier));
}

ctr_object *ctr_ast_set_value(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	char *cval = argumentList->object->value.svalue->value;
	size_t cvlen = argumentList->object->value.svalue->vlen;
	ctr_tnode *node = myself->value.rvalue->ptr;
	if (likely(node->vlen < cvlen))
		node->value = ctr_heap_reallocate(node->value, cvlen + 1);
	node->vlen = cvlen;
	memcpy(node->value, cval, cvlen);
	*(node->value + cvlen) = '\0';
	return myself;
}

ctr_object *ctr_ast_set_type(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	char *ctys = ctr_heap_allocate_cstring(argumentList->object);
	int ty = ctr_ast_tyfstr(ctys);
	ctr_heap_free(ctys);
	ctr_tnode *node = myself->value.rvalue->ptr;
	node->type = ty;
	return myself;
}

ctr_object *ctr_ast_set_mod(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	char *cmods = ctr_heap_allocate_cstring(argumentList->object);
	char mod = ctr_ast_modifier_fstr(cmods);
	ctr_heap_free(cmods);
	ctr_tnode *node = myself->value.rvalue->ptr;
	node->modifier = mod;
	return myself;
}

ctr_object *ctr_ast_evaluate(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	ctr_tnode *node = myself->value.rvalue->ptr;
	char ir;
	if (node->type == CTR_AST_NODE_PROGRAM)
		return ctr_cwlk_run(node);
	else
		return ctr_cwlk_expr(node, &ir);
}

char **str_split(char *a_str, const char a_delim, int *count_out)
{
	char **result = 0;
	size_t count = 0;
	char *tmp = a_str;
	char *last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;
	/* Count how many elements will be extracted. */
	while (*tmp) {
		if (a_delim == *tmp) {
			count++;
			last_comma = tmp;
		}
		tmp++;
	}
	count += last_comma < (a_str + strlen(a_str) - 1);
	count++;
	*count_out = count;
	result = malloc(sizeof(char *) * count);
	if (result) {
		size_t idx = 0;
		char *token = strsep(&a_str, delim);
		while (token) {
			// assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strsep(&a_str, delim);
		}
		// assert(idx == count - 1);
		*(result + idx) = 0;
	}
	return result;
}

char *ctr_ast_pure_stringify(ctr_tnode * node)
{
	char buf[2048];
	char *ret = NULL;
	if (node)
		switch (node->type) {
		case CTR_AST_NODE_EXPRASSIGNMENT:{
				char *rn =
				    ctr_ast_pure_stringify(node->nodes->node);
				char *rv =
				    ctr_ast_pure_stringify(node->nodes->next->
							   node);
				sprintf(buf, "%s is %s", rn, rv);
				ctr_heap_free(rn);
				ctr_heap_free(rv);
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_EXPRMESSAGE:{
				char *rr =
				    ctr_ast_pure_stringify(node->nodes->node);
				char *rm =
				    ctr_ast_pure_stringify(node->nodes->next->
							   node);
				sprintf(buf, "%s %s", rr, rm);
				ctr_heap_free(rr);
				ctr_heap_free(rm);
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_UNAMESSAGE:{
				size_t len = node->vlen;
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, node->value, len);
				break;
			}
		case CTR_AST_NODE_BINMESSAGE:{
				char *rr =
				    ctr_ast_pure_stringify(node->nodes->node);
				sprintf(buf, "%.*s %s", (int)(node->vlen),
					node->value, rr);
				ctr_heap_free(rr);
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_KWMESSAGE:{
				int msgpartc = 0;
				char *rm =
				    ctr_heap_allocate(sizeof(char) *
						      (node->vlen + 1));
				strncpy(rm, node->value, node->vlen);
				*(rm + node->vlen) = 0;
				ctr_tlistitem *partnodes = node->nodes;
				char **parts = str_split(rm, ':', &msgpartc);
				int x = 0;
				for (int i = 0; i < msgpartc - 1; i++) {
					char *rr =
					    ctr_ast_pure_stringify(partnodes->
								   node);
					x = sprintf(buf + x, "%s: %s ",
						    parts[i], rr);
					ctr_heap_free(rr);
					partnodes = partnodes->next;
				}
				ctr_heap_free(rm);
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_LTRSTRING:{
				ret =
				    ctr_heap_allocate(sizeof(char) *
						      (node->vlen + 3));
				ret[0] = '\'';
				memcpy(ret + 1, (node->value),
				       (node->vlen + 1));
				ret[node->vlen - 1] = '\'';
				ret[node->vlen] = 0;
				break;
			}
		case CTR_AST_NODE_REFERENCE:{
				switch (node->modifier) {
				case 1:{
						ret =
						    ctr_heap_allocate(sizeof
								      (char) *
								      (5 +
								       node->
								       vlen));
						sprintf(ret, "my %.*s",
							(int)(node->vlen),
							node->value);
						break;
					}
				case 2:{
						ret =
						    ctr_heap_allocate(sizeof
								      (char) *
								      (4 +
								       node->
								       vlen));
						sprintf(ret, "var %.*s",
							(int)(node->vlen),
							node->value);
						break;
					}
				case 3:{
						ret =
						    ctr_heap_allocate(sizeof
								      (char) *
								      (7 +
								       node->
								       vlen));
						sprintf(ret, "const %.*s",
							(int)(node->vlen),
							node->value);
						break;
					}
				default:{
						ret =
						    ctr_heap_allocate(sizeof
								      (char) *
								      (1 +
								       node->
								       vlen));
						sprintf(ret, "%.*s",
							(int)(node->vlen),
							node->value);
						break;
					}
				}
				break;
			}
		default:
		case CTR_AST_NODE_LTRNUM:{
				ret =
				    ctr_heap_allocate(sizeof(char) *
						      (node->vlen + 1));
				memcpy(ret, (node->value), (node->vlen + 1));
				break;
			}
		case CTR_AST_NODE_CODEBLOCK:{
				char *rparams =
				    ctr_ast_pure_stringify(node->nodes->node);
				char *rinstru =
				    ctr_ast_pure_stringify(node->nodes->next->
							   node);
				sprintf(buf, "{%s %s}", rparams, rinstru);
				ctr_heap_free(rparams);
				ctr_heap_free(rinstru);
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_RETURNFROMBLOCK:{
				char *rv =
				    ctr_ast_pure_stringify(node->nodes->node);
				int len = strlen(rv);
				ret =
				    ctr_heap_allocate(sizeof(char) * (len + 1));
				ret[0] = '^';
				memcpy(ret + 1, rv, len);
				ctr_heap_free(rv);
				break;
			}
		case CTR_AST_NODE_IMMUTABLE:{
				ctr_tlistitem *partnodes = node->nodes;
				int x = 0;
				while (partnodes) {
					char *rr =
					    ctr_ast_pure_stringify(partnodes->
								   node);
					if (rr) {
						x = sprintf(buf + x, "%s%s", rr,
							    partnodes->node->
							    nodes
							    && partnodes->node->
							    nodes->
							    next ? ", " : "");
						ctr_heap_free(rr);
					}
					if (!partnodes->node->nodes)
						break;
					partnodes =
					    partnodes->node->nodes->next;
				}
				int len = strlen(buf);
				ret =
				    ctr_heap_allocate(sizeof(char) * (len + 3));
				ret[0] = '[';
				memcpy(ret + 1, buf, len);
				ret[len + 1] = ']';
				ret[len + 2] = 0;
				break;
			}
		case CTR_AST_NODE_PARAMLIST:{
				ctr_tlistitem *partnodes = node->nodes;
				int x = 0;
				while (partnodes) {
					char *rr =
					    ctr_ast_pure_stringify(partnodes->
								   node);
					if (rr) {
						x = sprintf(buf + x, ":%s", rr);
						ctr_heap_free(rr);
					}
					partnodes = partnodes->next;
				}
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_INSTRLIST:{
				ctr_tlistitem *partnodes = node->nodes;
				int x = 0;
				while (partnodes) {
					char *rr =
					    ctr_ast_pure_stringify(partnodes->
								   node);
					x = sprintf(buf + x, "%s. ", rr);
					ctr_heap_free(rr);
					partnodes = partnodes->next;
				}
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		case CTR_AST_NODE_ENDOFPROGRAM:
			break;
		case CTR_AST_NODE_NESTED:{
				char *rv =
				    ctr_ast_pure_stringify(node->nodes->node);
				int len = strlen(rv);
				ret =
				    ctr_heap_allocate(sizeof(char) * (len + 2));
				ret[0] = '(';
				memcpy(ret + 1, rv, len);
				ret[len + 1] = ')';
				ctr_heap_free(rv);
				break;
			}
		case CTR_AST_NODE_LTRBOOLFALSE:
		case CTR_AST_NODE_LTRNIL:
		case CTR_AST_NODE_LTRBOOLTRUE:{
				char *rv =
				    ctr_ast_pure_stringify(node->nodes->node);
				int len = strlen(rv);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, rv, len);
				ctr_heap_free(rv);
				break;
			}
		case CTR_AST_NODE_PROGRAM:{
				ctr_tlistitem *partnodes = node->nodes;
				int x = 0;
				while (partnodes) {
					char *rr =
					    ctr_ast_pure_stringify(partnodes->
								   node);
					if (rr) {
						x = sprintf(buf + x, "%s. ",
							    rr);
						ctr_heap_free(rr);
					}
					partnodes = partnodes->next;
				}
				int len = strlen(buf);
				ret = ctr_heap_allocate(sizeof(char) * len);
				memcpy(ret, buf, len);
				break;
			}
		}
	(void)printf("[%p] %s :: %s\n", node, ctr_ast_tystr(node), ret);
	return ret;
}

ctr_object *ctr_ast_stringify(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdAst)
		return CtrStdNil;
	if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
		CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
		return CtrStdNil;
	}
	ctr_tnode *node = myself->value.rvalue->ptr;
	char *cstr = ctr_ast_pure_stringify(node);
	ctr_object *str = ctr_build_string_from_cstring(cstr);
	ctr_heap_free(cstr);
	return str;
}

ctr_object *CtrStdCoro;
ctr_object *CtrStdCoro_co;
void *ctr_coroutine_close(struct schedule *S)
{
	coroutine_close(S);
	return NULL;
}

ctr_object* ctr_ast_flex(ctr_object* myself, ctr_argument* argumentList) {
	ctr_lex_load(argumentList->object->value.svalue->value, argumentList->object->value.svalue->vlen);
	return myself;
}
static char* ctr_lex_token_lookup(int tok) {
	switch (tok) {
		case CTR_TOKEN_REF: return "REF";
		case CTR_TOKEN_QUOTE: return "QUOTE";
		case CTR_TOKEN_NUMBER: return "NUMBER";
		case CTR_TOKEN_PAROPEN: return "PAROPEN";
		case CTR_TOKEN_PARCLOSE: return "PARCLOSE";
		case CTR_TOKEN_BLOCKOPEN: return "BLOCKOPEN";
		case CTR_TOKEN_BLOCKOPEN_MAP: return "BLOCKOPEN_MAP";
		case CTR_TOKEN_BLOCKCLOSE: return "BLOCKCLOSE";
		case CTR_TOKEN_COLON: return "COLON";
		case CTR_TOKEN_DOT: return "DOT";
		case CTR_TOKEN_CHAIN: return "CHAIN";
		case CTR_TOKEN_BOOLEANYES: return "BOOLEANYES";
		case CTR_TOKEN_BOOLEANNO: return "BOOLEANNO";
		case CTR_TOKEN_NIL: return "NIL";
		case CTR_TOKEN_ASSIGNMENT: return "ASSIGNMENT";
		case CTR_TOKEN_RET: return "RET";
		case CTR_TOKEN_TUPOPEN: return "TUPOPEN";
		case CTR_TOKEN_TUPCLOSE: return "TUPCLOSE";
		case CTR_TOKEN_PASSIGNMENT: return "PASSIGNMENT";
		case CTR_TOKEN_FIN: return "FIN";
		default: return "UNKNOWN";
	}
}
ctr_object* ctr_ast_lextoken(ctr_object* myself, ctr_argument* argumentList) {
	int tok = ctr_lex_tok();
	char* toktype = ctr_lex_token_lookup(tok);
	char* value = ctr_lex_tok_value();
	size_t len = ctr_lex_tok_value_length();
	unsigned long pos = ctr_lex_position();
	ctr_argument* arg = ctr_heap_allocate(sizeof(*arg));

	ctr_object* obj = ctr_array_new(CtrStdArray, NULL);
	ctr_object* tokType = ctr_build_string_from_cstring(toktype);
	arg->object = tokType;
	ctr_array_push(obj, arg);
	tokType = ctr_build_string(value, len);
	arg->object = tokType;
	ctr_array_push(obj, arg);
	arg->object = ctr_build_number_from_float(pos);
	ctr_array_push(obj, arg);
	ctr_heap_free(arg);
	return obj;
}

ctr_object* ctr_ast_lexstring(ctr_object* myself, ctr_argument* argumentList) {
	char* toktype = "STRING";
	char* value = ctr_lex_readstr();
	size_t len = ctr_lex_tok_value_length();
	unsigned long pos = ctr_lex_position();
	ctr_argument* arg = ctr_heap_allocate(sizeof(*arg));

	ctr_object* obj = ctr_array_new(CtrStdArray, NULL);
	ctr_object* tokType = ctr_build_string_from_cstring(toktype);
	arg->object = tokType;
	ctr_array_push(obj, arg);
	tokType = ctr_build_string(value, len);
	arg->object = tokType;
	ctr_array_push(obj, arg);
	arg->object = ctr_build_number_from_float(pos);
	ctr_array_push(obj, arg);
	ctr_heap_free(arg);
	return obj;
}

ctr_object* ctr_ast_lexbuf(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* obj = ctr_lex_get_buf_str();
	return obj;
}
ctr_object* ctr_ast_lexpos(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size value = ctr_lex_position();
	ctr_object* obj = ctr_build_number_from_float(value);
	return obj;
}
ctr_object* ctr_ast_lexputback(ctr_object* myself, ctr_argument* argumentList) {
	ctr_lex_putback();
	return myself;
}
ctr_object* ctr_ast_lexskip(ctr_object* myself, ctr_argument* argumentList) {
	int len = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	ctr_lex_skip(len);
	return myself;
}

struct ctr_coro_args_data {
	ctr_object *co;
	ctr_object *block;
	ctr_argument *argumentList;
};
struct ctr_coro_coroutine {
	ctr_object *scheduler;
	ctr_object *context;
	int context_id;
	int co;
	int first;
};
void ctr_coroutine_run_block(struct schedule *S, void *ud)
{
	struct ctr_coro_args_data *data = ud;
	ctr_object *co = data->co;
	ctr_object *block = data->block;
	ctr_argument *args = data->argumentList;
	ctr_block_run(block, args, co);
}

/**
 * [Coroutine] new
 *
 * Makes a new coroutine scheduler
 */
ctr_object *ctr_coro_make(ctr_object * myself, ctr_argument * argumentList)
{
	ctr_object *coro = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	coro->link = CtrStdCoro;
	coro->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
	coro->value.rvalue->ptr = coroutine_open();
	coro->release_hook = (voidptrfn_t *) ctr_coroutine_close;
	return coro;
}

/**
 * [Coroutine] from: [Block] [arguments: [Array]]
 *
 * Makes a new coroutine object from the provided block, passing it the provided arguments
 */
ctr_object *ctr_coro_new(ctr_object * myself, ctr_argument * argumentList)
{
	if (myself == CtrStdCoro || !myself->value.rvalue
	    || !myself->value.rvalue->ptr) {
		CtrStdFlow =
		    ctr_build_string_from_cstring
		    ("No scheduler is provided. Initialize one with CtrStdCoro#new");
		return CtrStdNil;
	}
	struct schedule *S = myself->value.rvalue->ptr;
	struct ctr_coro_args_data *args = ctr_heap_allocate(sizeof(*args));
	ctr_object *coro = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	coro->link = CtrStdCoro_co;
	args->co = coro;
	args->block = argumentList->object;
	args->argumentList =
	    ctr_array_to_argument_list(argumentList->next ? argumentList->next->
				       object : NULL, NULL);
	int co = coroutine_new(S, ctr_coroutine_run_block, args);
	struct ctr_coro_coroutine *sco = ctr_heap_allocate(sizeof(*sco));
	sco->scheduler = myself;
	sco->co = co;
	sco->context = ctr_contexts[ctr_context_id];	//ctr_heap_allocate(sizeof(ctr_object*));
	// memmove(sco->context, ctr_contexts[ctr_context_id], sizeof(ctr_object));
	sco->context_id = ctr_context_id;
	sco->first = 1;
	coro->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
	coro->value.rvalue->ptr = sco;
	return coro;
}

ctr_object *ctr_coro_error(ctr_object * myself, ctr_argument * argList)
{
	CtrStdFlow =
	    ctr_build_string_from_cstring
	    ("Coroutine error. Probably calling co#new");
	return myself;
}

/**
 * [Coroutine#co] await
 *
 * resumes the coroutine block and returns the yielded value (or the coroutine object if nothing was yielded)
 */
ctr_object *ctr_coro_resume(ctr_object * myself, ctr_argument * argumentList)
{
	struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
	int co = sco->co;
	struct schedule *S = sco->scheduler->value.rvalue->ptr;
	if (!coroutine_status(S, co)) {
		// ctr_heap_free(sco->context);
		CtrStdFlow =
		    ctr_build_string_from_cstring("Coroutine not runnning");
		return myself;
	}
	if (!sco->first) {
		int last_context_id = ctr_context_id;
		ctr_context_id = sco->context_id;
		ctr_object *last_context = ctr_contexts[ctr_context_id];
		ctr_contexts[ctr_context_id] = sco->context;
		coroutine_resume(S, co);
		ctr_contexts[ctr_context_id] = last_context;
		ctr_context_id = last_context_id;
	} else {
		sco->first = 0;
		coroutine_resume(S, co);
		sco->context = ctr_contexts[ctr_context_id];
		sco->context_id = ctr_context_id;
		ctr_context_id = sco->context_id;
		ctr_contexts[ctr_context_id] = sco->context;
	}
	ctr_object *yielded =
	    ctr_internal_object_find_property(myself,
					      ctr_build_string_from_cstring
					      ("yield"), 0);
	return yielded ? yielded : myself;
}

/**
 * [Coroutine#co] yield[: [Object]]
 *
 * Yields the processor and optionally passes a value along
 */
ctr_object *ctr_coro_yield(ctr_object * myself, ctr_argument * argumentList)
{
	struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
	int co = sco->co;
	struct schedule *S = sco->scheduler->value.rvalue->ptr;
	if (coroutine_status(S, co) == COROUTINE_DEAD) {
		CtrStdFlow =
		    ctr_build_string_from_cstring("Coroutine not runnning");
		return myself;
	}
	coroutine_yield(S);
	if (argumentList->object)
		ctr_internal_object_set_property(myself,
						 ctr_build_string_from_cstring
						 ("yield"),
						 argumentList->object, 0);
	return myself;
}

ctr_object *ctr_coro_state(ctr_object * myself, ctr_argument * argumentList)
{
	struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
	int co = sco->co;
	struct schedule *S = sco->scheduler->value.rvalue->ptr;
	switch (coroutine_status(S, co)) {
	case COROUTINE_READY:
		return ctr_build_string_from_cstring("Ready");
	case COROUTINE_DEAD:
		return ctr_build_string_from_cstring("Finished");
	case COROUTINE_SUSPEND:
		return ctr_build_string_from_cstring("Suspended");
	case COROUTINE_RUNNING:
		return ctr_build_string_from_cstring("Running");
	}
	return CtrStdNil;
}

ctr_object *ctr_coro_isrunning(ctr_object * myself, ctr_argument * argumentList)
{
	struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
	int co = sco->co;
	struct schedule *S = sco->scheduler->value.rvalue->ptr;
	return ctr_build_bool(! !coroutine_status(S, co));
}

void initiailize_base_extensions()
{
	ctr_internal_create_func(CtrStdObject,
				 ctr_build_string_from_cstring("letEqual:in:"),
				 &ctr_block_let);
	CtrStdAst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	CtrStdAst->link = CtrStdObject;
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("instrCount"),
				 &ctr_ast_instrcount);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("count"),
				 &ctr_ast_instrcount);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("parse:"),
				 &ctr_ast_parse);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("at:"),
				 &ctr_ast_nth);
	ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("@"),
				 &ctr_ast_nth);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("toString"),
				 &ctr_ast_tostring);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("value"),
				 &ctr_ast_get_value);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("type"),
				 &ctr_ast_get_type);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("modifier"),
				 &ctr_ast_get_mod);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("value:"),
				 &ctr_ast_set_value);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("type:"),
				 &ctr_ast_set_type);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("modifier:"),
				 &ctr_ast_set_mod);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("evaluate"),
				 &ctr_ast_evaluate);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("unparse"),
				 &ctr_ast_stringify);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("feedLexer:"),
				 &ctr_ast_flex);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("token"),
				 &ctr_ast_lextoken);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("lexstring"),
				 &ctr_ast_lexstring);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("getlexbuf"),
				 &ctr_ast_lexbuf);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("lexSkip:"),
				 &ctr_ast_lexskip);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("lexpos"),
				 &ctr_ast_lexpos);
	ctr_internal_create_func(CtrStdAst,
				 ctr_build_string_from_cstring("lexPutBack"),
				 &ctr_ast_lexputback);
	ctr_internal_object_add_property(CtrStdWorld,
					 ctr_build_string_from_cstring("AST"),
					 CtrStdAst, 0);

	CtrStdCoro = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdCoro->link = CtrStdObject;
	ctr_internal_create_func(CtrStdCoro,
				 ctr_build_string_from_cstring("new"),
				 &ctr_coro_make);
	ctr_internal_create_func(CtrStdCoro,
				 ctr_build_string_from_cstring("from:"),
				 &ctr_coro_new);
	ctr_internal_create_func(CtrStdCoro,
				 ctr_build_string_from_cstring
				 ("from:arguments:"), &ctr_coro_new);
	ctr_internal_object_add_property(CtrStdWorld,
					 ctr_build_string_from_cstring
					 ("CoroutineScheduler"), CtrStdCoro, 0);
	ctr_object *ctrstdcoro_default_sched = ctr_coro_make(CtrStdCoro, NULL);
	ctr_internal_object_add_property(CtrStdWorld,
					 ctr_build_string_from_cstring
					 ("Coroutine"),
					 ctrstdcoro_default_sched, 0);
	CtrStdCoro_co = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdCoro_co->link = CtrStdObject;
	ctr_internal_create_func(CtrStdCoro_co,
				 ctr_build_string_from_cstring("await"),
				 &ctr_coro_resume);
	ctr_internal_create_func(CtrStdCoro_co,
				 ctr_build_string_from_cstring("run"),
				 &ctr_coro_resume);
	ctr_internal_create_func(CtrStdCoro_co,
				 ctr_build_string_from_cstring("yield"),
				 &ctr_coro_yield);
	ctr_internal_create_func(CtrStdCoro_co,
				 ctr_build_string_from_cstring("yield:"),
				 &ctr_coro_yield);
	ctr_internal_create_func(CtrStdCoro_co,
				 ctr_build_string_from_cstring("isRunning"),
				 &ctr_coro_isrunning);
	ctr_internal_create_func(CtrStdCoro_co,
				 ctr_build_string_from_cstring("state"),
				 &ctr_coro_state);

}
