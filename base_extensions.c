#include "citron.h"
#include "coroutine.h"
#include "generator.h"
#include "symbol.h"
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * [Array|String] letEqual: [Array|Object] in: [Block]
 *
 * Assigns local values to the block, and runs it as a closure
 */
ctr_object *ctr_block_let(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *defs = argumentList->object, *block = argumentList->next->object;
  ctr_object *result;
  ctr_open_context();
  if (myself->info.type == CTR_OBJECT_TYPE_OTARRAY) {
    for (ctr_size i = 0;
         i < myself->value.avalue->head - myself->value.avalue->tail &&
         (defs->info.type == CTR_OBJECT_TYPE_OTARRAY &&
          i < defs->value.avalue->head - defs->value.avalue->tail);
         i++) {
      switch (defs->info.type) {
      case CTR_OBJECT_TYPE_OTARRAY:
        ctr_assign_value_to_local(myself->value.avalue->elements[i],
                                  defs->value.avalue->elements[i]);
        break;
      default:
        ctr_assign_value_to_local(myself->value.avalue->elements[i], defs);
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

ctr_object *ctr_reown_obj(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *owned = argumentList->object;
  ctr_object *newowner = argumentList->next->object;
  if (myself->properties->size == 0)
    goto exit_not_ours;
  ctr_mapitem *my_properties = myself->properties->head;
  int found = 0;
  while (my_properties) {
    if (my_properties->value == owned) {
      found = 1;
      ctr_internal_object_add_property_with_hash(newowner, my_properties->key,
                                                 my_properties->hashKey,
                                                 my_properties->value, 0);
      ctr_internal_object_delete_property_with_hash(myself, my_properties->key,
                                                    my_properties->hashKey, 0);
      break;
    }
    my_properties = my_properties->next;
  }
  if (!found)
    goto exit_not_ours;
  return myself;
exit_not_ours:;
  CtrStdFlow = ctr_build_string_from_cstring(
      "Attempt to transfer ownership of an object that we do not own");
  return myself;
}
ctr_object *CtrStdAst;
/**
 * [AST] parse: [String]
 *
 * parse string into citron ast
 */
ctr_object *ctr_ast_parse(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *psec = argumentList->object;
  char *prg = ctr_heap_allocate_cstring(psec);
  int last_q = ctr_cparse_quiet;
  ctr_cparse_quiet = 1;
  ctr_program_length = psec->value.svalue->vlen;
  ctr_tnode *ped = ctr_cparse_parse(prg, "ASTparse");
  ctr_cparse_quiet = last_q;
  ctr_object *ast = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  // ctr_set_link_all(ast, CtrStdAst);
  ctr_set_link_all(ast, CtrStdAst);
  ast->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ast->value.rvalue->ptr = ped;
  ast->value.rvalue->type = CTR_AST_TYPE;
  return ast;
}

ctr_object *ctr_ast_instrcount(ctr_object *myself, ctr_argument *argumentList) {
  // if (myself == CtrStdAst) return ctr_build_number_from_float(0);
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
    return CtrStdNil;
  }
  ctr_tnode *node = myself->value.rvalue->ptr;
  int count = 0;
  ctr_tlistitem *pitem = node->nodes;
  while (pitem) {
    count += pitem->node->type != CTR_AST_NODE_ENDOFPROGRAM;
    pitem = pitem->next;
  }
  return ctr_build_number_from_float(count);
}

ctr_object *ctr_ast_nth(ctr_object *myself, ctr_argument *argumentList) {
  if (myself == CtrStdAst)
    return CtrStdNil;
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
  err:;
    CtrStdFlow =
        ctr_build_string_from_cstring("Null ast node or count out of range");
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
  ctr_set_link_all(ast, CtrStdAst);
  ast->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ast->value.rvalue->ptr = pitem->node;
  ast->value.rvalue->type = CTR_AST_TYPE;
  return ast;
}

ctr_object *ctr_ast_insert_nth(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *tnodeobj = argumentList->object;
  int nullast = 0, ip = 0;
  if (myself == CtrStdAst)
    return CtrStdNil;
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    nullast = 1;
  err:;
    CtrStdFlow = ctr_build_string_from_cstring(
        ip ? "incorrect passed argument"
           : (nullast ? "Null ast node" : "count out of range"));
    return CtrStdNil;
  }
  if (tnodeobj->value.rvalue->type != CTR_AST_TYPE) {
    ip = 1;
    goto err;
  }
  ctr_tnode *insnode = tnodeobj->value.rvalue->ptr;
  if (!insnode)
    goto err;
  int n = argumentList->next->object->value.nvalue;
  ctr_tnode *node = myself->value.rvalue->ptr;
  ctr_tlistitem *pitem = node->nodes, *oldpitem = pitem;
  while (n > 0) {
    if (!pitem)
      goto err;
    n--;
    oldpitem = pitem;
    pitem = pitem->next;
  }
  if (!pitem && n > -1)
    goto err;
  if (oldpitem != pitem) {
    ctr_tlistitem *insitem = ctr_heap_allocate(sizeof(ctr_tlistitem));
    insitem->node = insnode;
    insitem->next = pitem->next;
    pitem->next = insitem;
  } else {
    ctr_tlistitem *insitem = ctr_heap_allocate(sizeof(ctr_tlistitem));
    insitem->node = insnode;
    insitem->next = pitem;
    node->nodes = insitem;
  }
  return myself;
}
ctr_object *ctr_ast_set_nth(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *tnodeobj = argumentList->object;
  int nullast = 0, ip = 0;
  if (myself == CtrStdAst)
    return CtrStdNil;
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    nullast = 1;
  err:;
    CtrStdFlow = ctr_build_string_from_cstring(
        ip ? "incorrect passed argument"
           : (nullast ? "Null ast node" : "count out of range"));
    return CtrStdNil;
  }
  if (tnodeobj->value.rvalue->type != CTR_AST_TYPE) {
    ip = 1;
    goto err;
  }
  ctr_tnode *insnode = tnodeobj->value.rvalue->ptr;
  if (!insnode)
    goto err;
  int n = argumentList->next->object->value.nvalue;
  ctr_tnode *node = myself->value.rvalue->ptr;
  ctr_tlistitem *pitem = node->nodes, *oldpitem = pitem;
  while (n > 0) {
    if (!pitem)
      goto err;
    n--;
    oldpitem = pitem;
    pitem = pitem->next;
  }
  if (!pitem)
    goto err;
  pitem->node = insnode;
  return myself;
}

ctr_object *ctr_ast_each(ctr_object *myself, ctr_argument *argumentList) {
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
    return CtrStdNil;
  }
  ctr_object *blk = argumentList->object;
  ctr_tnode *node = myself->value.rvalue->ptr;
  ctr_tlistitem *pitem = node->nodes;
  ctr_size count = 0;
  argumentList->next = ctr_heap_allocate(sizeof(ctr_argument));
  while (pitem) {
    if (CtrStdFlow) {
      if (CtrStdFlow == CtrStdContinue) {
        CtrStdFlow = NULL;
        continue;
      }
      if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
      break;
    }
    if (pitem->node->type != CTR_AST_NODE_ENDOFPROGRAM) {
      argumentList->object = ctr_build_number_from_float(count);
      argumentList->next->object = ctr_ast_nth(myself, argumentList);
      count++;
      ctr_block_run_here(blk, argumentList, myself);
    }
    pitem = pitem->next;
  }
  ctr_heap_free(argumentList->next);
  argumentList->next = NULL;
  argumentList->object = blk;
  return myself;
}

ctr_object *ctr_ast_eachv(ctr_object *myself, ctr_argument *argumentList) {
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
    return CtrStdNil;
  }
  ctr_object *blk = argumentList->object;
  ctr_tnode *node = myself->value.rvalue->ptr;
  ctr_tlistitem *pitem = node->nodes;
  ctr_size count = 0;
  while (pitem) {
    if (CtrStdFlow) {
      if (CtrStdFlow == CtrStdContinue) {
        CtrStdFlow = NULL;
        continue;
      }
      if (CtrStdFlow == CtrStdBreak)
        CtrStdFlow = NULL;
      break;
    }
    if (pitem->node->type != CTR_AST_NODE_ENDOFPROGRAM) {
      argumentList->object = ctr_build_number_from_float(count);
      argumentList->object = ctr_ast_nth(myself, argumentList);
      count++;
      ctr_block_run_here(blk, argumentList, myself);
    }
    pitem = pitem->next;
  }
  argumentList->object = blk;
  return myself;
}

static char const *ctr_ast_tystr(ctr_tnode *ast) {
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
  case CTR_AST_NODE_SYMBOL:
    return "SYMBOL";
  default:
    return "UNKNOWN";
  }
}

static char const *ctr_ast_modifier_str(char modifier) {
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

static int ctr_ast_tyfstr(char *type) {
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
  if (strcasecmp("SYMBOL", type) == 0)
    return CTR_AST_NODE_SYMBOL;
  return CTR_AST_NODE_ENDOFPROGRAM;
}

static char ctr_ast_modifier_fstr(char *mod) {
  if (strcasecmp("My", mod) == 0)
    return 1;
  if (strcasecmp("Var", mod) == 0)
    return 2;
  if (strcasecmp("Const", mod) == 0)
    return 3;
  return 0;
}

ctr_object *ctr_ast_tostring(ctr_object *myself, ctr_argument *argumentList) {
  char buf[1024];
  ctr_tnode *ptr = myself->value.rvalue ? myself->value.rvalue->ptr : NULL;
  (void)sprintf(buf, "<AST:%s<%.*s> at %p>", ctr_ast_tystr(ptr),
                (int)(ptr ? ptr->vlen : 5), ptr ? ptr->value : "(nil)", ptr);
  return ctr_build_string_from_cstring(buf);
}

ctr_object *ctr_ast_get_value(ctr_object *myself, ctr_argument *argumentList) {
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

ctr_object *ctr_ast_get_type(ctr_object *myself, ctr_argument *argumentList) {
  if (myself == CtrStdAst)
    return CtrStdNil;
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
    return CtrStdNil;
  }
  ctr_tnode *node = myself->value.rvalue->ptr;
  return ctr_build_string_from_cstring((char *)ctr_ast_tystr(node));
}

ctr_object *ctr_ast_get_mod(ctr_object *myself, ctr_argument *argumentList) {
  if (myself == CtrStdAst)
    return CtrStdNil;
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
    return CtrStdNil;
  }
  ctr_tnode *node = myself->value.rvalue->ptr;
  return ctr_build_string_from_cstring(
      (char *)ctr_ast_modifier_str(node->modifier));
}

ctr_object *ctr_ast_set_value(ctr_object *myself, ctr_argument *argumentList) {
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

ctr_object *ctr_ast_with_value(ctr_object *myself, ctr_argument *argumentList) {
  ctr_tnode *node = ctr_heap_allocate(sizeof(ctr_tnode));
  ctr_string *str = argumentList->object->value.svalue;
  node->value = str->value;
  node->vlen = str->vlen;
  ctr_object *ast = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(ast, CtrStdAst);
  ast->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  ast->value.rvalue->ptr = node;
  ast->value.rvalue->type = CTR_AST_TYPE;
  return ast;
}

ctr_object *ctr_ast_set_type(ctr_object *myself, ctr_argument *argumentList) {
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

ctr_object *ctr_ast_set_mod(ctr_object *myself, ctr_argument *argumentList) {
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

ctr_object *ctr_ast_evaluate(ctr_object *myself, ctr_argument *argumentList) {
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

char **str_split(char *a_str, const char a_delim, int *count_out) {
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

char *ctr_ast_pure_stringify(ctr_tnode *node) {
  char buf[2048];
  char *ret = NULL;
  if (node)
    switch (node->type) {
    case CTR_AST_NODE_EXPRASSIGNMENT: {
      char *rn = ctr_ast_pure_stringify(node->nodes->node);
      char *rv = ctr_ast_pure_stringify(node->nodes->next->node);
      sprintf(buf, "%s is %s", rn, rv);
      ctr_heap_free(rn);
      ctr_heap_free(rv);
      int len = strlen(buf);
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, buf, len);
      break;
    }
    case CTR_AST_NODE_EXPRMESSAGE: {
      char *rr = ctr_ast_pure_stringify(node->nodes->node);
      char *rm = ctr_ast_pure_stringify(node->nodes->next->node);
      sprintf(buf, "%s %s", rr, rm);
      ctr_heap_free(rr);
      ctr_heap_free(rm);
      int len = strlen(buf);
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, buf, len);
      break;
    }
    case CTR_AST_NODE_UNAMESSAGE: {
      size_t len = node->vlen;
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, node->value, len);
      break;
    }
    case CTR_AST_NODE_BINMESSAGE: {
      char *rr = ctr_ast_pure_stringify(node->nodes->node);
      sprintf(buf, "%.*s %s", (int)(node->vlen), node->value, rr);
      ctr_heap_free(rr);
      int len = strlen(buf);
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, buf, len);
      break;
    }
    case CTR_AST_NODE_KWMESSAGE: {
      int msgpartc = 0;
      char *rm = ctr_heap_allocate(sizeof(char) * (node->vlen + 1));
      strncpy(rm, node->value, node->vlen);
      *(rm + node->vlen) = 0;
      ctr_tlistitem *partnodes = node->nodes;
      char **parts = str_split(rm, ':', &msgpartc);
      int x = 0;
      for (int i = 0; i < msgpartc - 1; i++) {
        char *rr = ctr_ast_pure_stringify(partnodes->node);
        x = sprintf(buf + x, "%s: %s ", parts[i], rr);
        ctr_heap_free(rr);
        partnodes = partnodes->next;
      }
      ctr_heap_free(rm);
      int len = strlen(buf);
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, buf, len);
      break;
    }
    case CTR_AST_NODE_SYMBOL:
    case CTR_AST_NODE_LTRSTRING: {
      ret = ctr_heap_allocate(sizeof(char) * (node->vlen + 3));
      ret[0] = '\'';
      memcpy(ret + 1, (node->value), (node->vlen + 1));
      ret[node->vlen - 1] = '\'';
      ret[node->vlen] = 0;
      break;
    }
    case CTR_AST_NODE_REFERENCE: {
      switch (node->modifier) {
      case 1: {
        ret = ctr_heap_allocate(sizeof(char) * (5 + node->vlen));
        sprintf(ret, "my %.*s", (int)(node->vlen), node->value);
        break;
      }
      case 2: {
        ret = ctr_heap_allocate(sizeof(char) * (4 + node->vlen));
        sprintf(ret, "var %.*s", (int)(node->vlen), node->value);
        break;
      }
      case 3: {
        ret = ctr_heap_allocate(sizeof(char) * (7 + node->vlen));
        sprintf(ret, "const %.*s", (int)(node->vlen), node->value);
        break;
      }
      default: {
        ret = ctr_heap_allocate(sizeof(char) * (1 + node->vlen));
        sprintf(ret, "%.*s", (int)(node->vlen), node->value);
        break;
      }
      }
      break;
    }
    default:
    case CTR_AST_NODE_LTRNUM: {
      ret = ctr_heap_allocate(sizeof(char) * (node->vlen + 1));
      memcpy(ret, (node->value), (node->vlen + 1));
      break;
    }
    case CTR_AST_NODE_CODEBLOCK: {
      char *rparams = ctr_ast_pure_stringify(node->nodes->node);
      char *rinstru = ctr_ast_pure_stringify(node->nodes->next->node);
      sprintf(buf, "{%s %s}", rparams, rinstru);
      ctr_heap_free(rparams);
      ctr_heap_free(rinstru);
      int len = strlen(buf);
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, buf, len);
      break;
    }
    case CTR_AST_NODE_RETURNFROMBLOCK: {
      char *rv = ctr_ast_pure_stringify(node->nodes->node);
      int len = strlen(rv);
      ret = ctr_heap_allocate(sizeof(char) * (len + 1));
      ret[0] = '^';
      memcpy(ret + 1, rv, len);
      ctr_heap_free(rv);
      break;
    }
    case CTR_AST_NODE_IMMUTABLE: {
      ctr_tlistitem *partnodes = node->nodes;
      int x = 0;
      while (partnodes) {
        char *rr = ctr_ast_pure_stringify(partnodes->node);
        if (rr) {
          x = sprintf(buf + x, "%s%s", rr,
                      partnodes->node->nodes && partnodes->node->nodes->next
                          ? ", "
                          : "");
          ctr_heap_free(rr);
        }
        if (!partnodes->node->nodes)
          break;
        partnodes = partnodes->node->nodes->next;
      }
      int len = strlen(buf);
      ret = ctr_heap_allocate(sizeof(char) * (len + 3));
      ret[0] = '[';
      memcpy(ret + 1, buf, len);
      ret[len + 1] = ']';
      ret[len + 2] = 0;
      break;
    }
    case CTR_AST_NODE_PARAMLIST: {
      ctr_tlistitem *partnodes = node->nodes;
      int x = 0;
      while (partnodes) {
        char *rr = ctr_ast_pure_stringify(partnodes->node);
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
    case CTR_AST_NODE_INSTRLIST: {
      ctr_tlistitem *partnodes = node->nodes;
      int x = 0;
      while (partnodes) {
        char *rr = ctr_ast_pure_stringify(partnodes->node);
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
    case CTR_AST_NODE_NESTED: {
      char *rv = ctr_ast_pure_stringify(node->nodes->node);
      int len = strlen(rv);
      ret = ctr_heap_allocate(sizeof(char) * (len + 2));
      ret[0] = '(';
      memcpy(ret + 1, rv, len);
      ret[len + 1] = ')';
      ctr_heap_free(rv);
      break;
    }
    case CTR_AST_NODE_LTRBOOLFALSE:
    case CTR_AST_NODE_LTRNIL:
    case CTR_AST_NODE_LTRBOOLTRUE: {
      char *rv = ctr_ast_pure_stringify(node->nodes->node);
      int len = strlen(rv);
      ret = ctr_heap_allocate(sizeof(char) * len);
      memcpy(ret, rv, len);
      ctr_heap_free(rv);
      break;
    }
    case CTR_AST_NODE_PROGRAM: {
      ctr_tlistitem *partnodes = node->nodes;
      int x = 0;
      while (partnodes) {
        char *rr = ctr_ast_pure_stringify(partnodes->node);
        if (rr) {
          x = sprintf(buf + x, "%s. ", rr);
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

ctr_object *ctr_ast_stringify(ctr_object *myself, ctr_argument *argumentList) {
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
void *ctr_coroutine_close(struct schedule *S) {
  coroutine_close(S);
  return NULL;
}

ctr_object *ctr_ast_flex(ctr_object *myself, ctr_argument *argumentList) {
  ctr_lex_load(argumentList->object->value.svalue->value,
               argumentList->object->value.svalue->vlen);
  return myself;
}
static char *ctr_lex_token_lookup(int tok) {
  switch (tok) {
  case CTR_TOKEN_REF:
    return "REF";
  case CTR_TOKEN_QUOTE:
    return "QUOTE";
  case CTR_TOKEN_NUMBER:
    return "NUMBER";
  case CTR_TOKEN_PAROPEN:
    return "PAROPEN";
  case CTR_TOKEN_PARCLOSE:
    return "PARCLOSE";
  case CTR_TOKEN_BLOCKOPEN:
    return "BLOCKOPEN";
  case CTR_TOKEN_BLOCKOPEN_MAP:
    return "BLOCKOPEN_MAP";
  case CTR_TOKEN_BLOCKCLOSE:
    return "BLOCKCLOSE";
  case CTR_TOKEN_COLON:
    return "COLON";
  case CTR_TOKEN_DOT:
    return "DOT";
  case CTR_TOKEN_CHAIN:
    return "CHAIN";
  case CTR_TOKEN_BOOLEANYES:
    return "BOOLEANYES";
  case CTR_TOKEN_BOOLEANNO:
    return "BOOLEANNO";
  case CTR_TOKEN_NIL:
    return "NIL";
  case CTR_TOKEN_ASSIGNMENT:
    return "ASSIGNMENT";
  case CTR_TOKEN_RET:
    return "RET";
  case CTR_TOKEN_TUPOPEN:
    return "TUPOPEN";
  case CTR_TOKEN_TUPCLOSE:
    return "TUPCLOSE";
  case CTR_TOKEN_PASSIGNMENT:
    return "PASSIGNMENT";
  case CTR_TOKEN_SYMBOL:
    return "SYMBOL";
  case CTR_TOKEN_LC_SEP:
    return "LC_SEP";
  case CTR_TOKEN_FIN:
    return "FIN";
  default:
    return "UNKNOWN";
  }
}
ctr_object *ctr_ast_lextoken(ctr_object *myself, ctr_argument *argumentList) {
  int tok = ctr_lex_tok();
  char *toktype = ctr_lex_token_lookup(tok);
  char *value = ctr_lex_tok_value();
  size_t len = ctr_lex_tok_value_length();
  unsigned long pos = ctr_lex_position();
  ctr_argument *arg = ctr_heap_allocate(sizeof(*arg));

  ctr_object *obj = ctr_array_new(CtrStdArray, NULL);
  ctr_object *tokType = ctr_build_string_from_cstring(toktype);
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

ctr_object *ctr_ast_lexstring(ctr_object *myself, ctr_argument *argumentList) {
  char *toktype = "STRING";
  char *value = ctr_lex_readstr();
  size_t len = ctr_lex_tok_value_length();
  unsigned long pos = ctr_lex_position();
  ctr_argument *arg = ctr_heap_allocate(sizeof(*arg));

  ctr_object *obj = ctr_array_new(CtrStdArray, NULL);
  ctr_object *tokType = ctr_build_string_from_cstring(toktype);
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

ctr_object *ctr_ast_lexbuf(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *obj = ctr_lex_get_buf_str();
  return obj;
}
ctr_object *ctr_ast_lexpos(ctr_object *myself, ctr_argument *argumentList) {
  ctr_size value = ctr_lex_position();
  ctr_object *obj = ctr_build_number_from_float(value);
  return obj;
}
ctr_object *ctr_ast_lexputback(ctr_object *myself, ctr_argument *argumentList) {
  ctr_lex_putback();
  return myself;
}
ctr_object *ctr_ast_marshal(ctr_object *myself, ctr_argument *argumentList) {
  if (myself == CtrStdAst)
    return CtrStdNil;
  if (!(myself->value.rvalue && myself->value.rvalue->ptr)) {
    CtrStdFlow = ctr_build_string_from_cstring("Null ast node");
    return CtrStdNil;
  }
  ctr_tnode *node = myself->value.rvalue->ptr;
  ctr_size avail = 1024, used = 0;
  char *buf = ctr_heap_allocate(avail);
  ctr_marshal_ast(node, &buf, &avail, &used);
  ctr_object *str = ctr_build_string(buf, used);
  ctr_heap_free(buf);
  return str;
}
ctr_object *ctr_ast_lexskip(ctr_object *myself, ctr_argument *argumentList) {
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
void ctr_coroutine_run_block(struct schedule *S, void *ud) {
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
ctr_object *ctr_coro_make(ctr_object *myself, ctr_argument *argumentList) {
  ctr_object *coro = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(coro, CtrStdCoro);
  coro->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  coro->value.rvalue->ptr = coroutine_open();
  coro->release_hook = (voidptrfn_t *)ctr_coroutine_close;
  return coro;
}

/**
 * [Coroutine] from: [Block] [arguments: [Array]]
 *
 * Makes a new coroutine object from the provided block, passing it the provided
 * arguments
 */
ctr_object *ctr_coro_new(ctr_object *myself, ctr_argument *argumentList) {
  if (myself == CtrStdCoro || !myself->value.rvalue ||
      !myself->value.rvalue->ptr) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "No scheduler is provided. Initialize one with CtrStdCoro#new");
    return CtrStdNil;
  }
  struct schedule *S = myself->value.rvalue->ptr;
  struct ctr_coro_args_data *args = ctr_heap_allocate(sizeof(*args));
  ctr_object *coro = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(coro, CtrStdCoro_co);
  args->co = coro;
  args->block = argumentList->object;
  args->argumentList = ctr_array_to_argument_list(
      argumentList->next ? argumentList->next->object : NULL, NULL);
  int co = coroutine_new(S, ctr_coroutine_run_block, args);
  struct ctr_coro_coroutine *sco = ctr_heap_allocate(sizeof(*sco));
  sco->scheduler = myself;
  sco->co = co;
  sco->context =
      ctr_contexts[ctr_context_id]; // ctr_heap_allocate(sizeof(ctr_object*));
  // memmove(sco->context, ctr_contexts[ctr_context_id], sizeof(ctr_object));
  sco->context_id = ctr_context_id;
  sco->first = 1;
  coro->value.rvalue = ctr_heap_allocate(sizeof(ctr_resource));
  coro->value.rvalue->ptr = sco;
  return coro;
}

ctr_object *ctr_coro_error(ctr_object *myself, ctr_argument *argList) {
  CtrStdFlow =
      ctr_build_string_from_cstring("Coroutine error. Probably calling co#new");
  return myself;
}

/**
 * [Coroutine#co] await
 *
 * resumes the coroutine block and returns the yielded value (or the coroutine
 * object if nothing was yielded)
 */
ctr_object *ctr_coro_resume(ctr_object *myself, ctr_argument *argumentList) {
  struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
  int co = sco->co;
  struct schedule *S = sco->scheduler->value.rvalue->ptr;
  if (!coroutine_status(S, co)) {
    // ctr_heap_free(sco->context);
    CtrStdFlow = ctr_build_string_from_cstring("Coroutine not runnning");
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
  ctr_object *yielded = ctr_internal_object_find_property(
      myself, ctr_build_string_from_cstring("yield"), 0);
  return yielded ? yielded : myself;
}

/**
 * [Coroutine#co] yield[: [Object]]
 *
 * Yields the processor and optionally passes a value along
 */
ctr_object *ctr_coro_yield(ctr_object *myself, ctr_argument *argumentList) {
  struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
  int co = sco->co;
  struct schedule *S = sco->scheduler->value.rvalue->ptr;
  if (coroutine_status(S, co) == COROUTINE_DEAD) {
    CtrStdFlow = ctr_build_string_from_cstring("Coroutine not runnning");
    return myself;
  }
  coroutine_yield(S);
  if (argumentList->object)
    ctr_internal_object_set_property(myself,
                                     ctr_build_string_from_cstring("yield"),
                                     argumentList->object, 0);
  return myself;
}

ctr_object *ctr_coro_state(ctr_object *myself, ctr_argument *argumentList) {
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

ctr_object *ctr_coro_isrunning(ctr_object *myself, ctr_argument *argumentList) {
  struct ctr_coro_coroutine *sco = myself->value.rvalue->ptr;
  int co = sco->co;
  struct schedule *S = sco->scheduler->value.rvalue->ptr;
  return ctr_build_bool(!!coroutine_status(S, co));
}

ctr_object *ctr_string_to_symbol(ctr_object *myself,
                                 ctr_argument *argumentList) {
  ctr_object *sym = ctr_get_or_create_symbol_table_entry(
      myself->value.svalue->value, myself->value.svalue->vlen);
  return sym;
}

ctr_object *ctr_object_inherit(ctr_object *myself, ctr_argument *argumentList) {
  if (myself->info.shared) {
    CtrStdFlow = ctr_build_string_from_cstring(
        "Cannot add interfaces to shared objects");
    return myself;
  }
  ctr_interfaces *ifsp = myself->interfaces;
  ctr_object *ifs = argumentList->object;
  for (int i = 0; i > ifsp->count; i++)
    if (ifsp->ifs[i] == ifs)
      return myself;
  ifsp->count++;
  ifsp->ifs =
      ctr_heap_reallocate(ifsp->ifs, sizeof(ctr_object *) * (ifsp->count + 1));
  ifsp->ifs[ifsp->count] = NULL;
  ifsp->ifs[ifsp->count - 1] = ifs;
  return myself;
}

void initiailize_base_extensions() {
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("letEqual:in:"),
                           &ctr_block_let);
  ctr_internal_create_func(
      CtrStdBlock, ctr_build_string_from_cstring("transferOwnershipOf:to:"),
      &ctr_reown_obj);
  CtrStdAst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  ctr_set_link_all(CtrStdAst, CtrStdObject);
  ctr_internal_create_func(CtrStdAst,
                           ctr_build_string_from_cstring("instrCount"),
                           &ctr_ast_instrcount);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("count"),
                           &ctr_ast_instrcount);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("parse:"),
                           &ctr_ast_parse);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("at:"),
                           &ctr_ast_nth);
  ctr_internal_create_func(CtrStdAst,
                           ctr_build_string_from_cstring("insert:at:"),
                           &ctr_ast_insert_nth);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("put:at:"),
                           &ctr_ast_set_nth);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("@"),
                           &ctr_ast_nth);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("each:"),
                           &ctr_ast_each);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("each_v:"),
                           &ctr_ast_eachv);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("toString"),
                           &ctr_ast_tostring);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("value"),
                           &ctr_ast_get_value);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("type"),
                           &ctr_ast_get_type);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("modifier"),
                           &ctr_ast_get_mod);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("value:"),
                           &ctr_ast_set_value);
  ctr_internal_create_func(CtrStdAst,
                           ctr_build_string_from_cstring("withValue:"),
                           &ctr_ast_with_value);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("type:"),
                           &ctr_ast_set_type);
  ctr_internal_create_func(
      CtrStdAst, ctr_build_string_from_cstring("modifier:"), &ctr_ast_set_mod);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("evaluate"),
                           &ctr_ast_evaluate);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("unparse"),
                           &ctr_ast_stringify);
  ctr_internal_create_func(
      CtrStdAst, ctr_build_string_from_cstring("feedLexer:"), &ctr_ast_flex);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("token"),
                           &ctr_ast_lextoken);
  ctr_internal_create_func(CtrStdAst,
                           ctr_build_string_from_cstring("lexstring"),
                           &ctr_ast_lexstring);
  ctr_internal_create_func(
      CtrStdAst, ctr_build_string_from_cstring("getlexbuf"), &ctr_ast_lexbuf);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("lexSkip:"),
                           &ctr_ast_lexskip);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("lexpos"),
                           &ctr_ast_lexpos);
  ctr_internal_create_func(CtrStdAst,
                           ctr_build_string_from_cstring("lexPutBack"),
                           &ctr_ast_lexputback);
  ctr_internal_create_func(CtrStdAst, ctr_build_string_from_cstring("marshal"),
                           &ctr_ast_marshal);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("AST"), CtrStdAst, 0);

  CtrStdCoro = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdCoro, CtrStdObject);
  ctr_internal_create_func(CtrStdCoro, ctr_build_string_from_cstring("new"),
                           &ctr_coro_make);
  ctr_internal_create_func(CtrStdCoro, ctr_build_string_from_cstring("from:"),
                           &ctr_coro_new);
  ctr_internal_create_func(CtrStdCoro,
                           ctr_build_string_from_cstring("from:arguments:"),
                           &ctr_coro_new);
  ctr_internal_object_add_property(
      CtrStdWorld, ctr_build_string_from_cstring("CoroutineScheduler"),
      CtrStdCoro, 0);
  ctr_object *ctrstdcoro_default_sched = ctr_coro_make(CtrStdCoro, NULL);
  ctr_internal_object_add_property(CtrStdWorld,
                                   ctr_build_string_from_cstring("Coroutine"),
                                   ctrstdcoro_default_sched, 0);
  CtrStdCoro_co = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdCoro_co, CtrStdObject);
  ctr_internal_create_func(
      CtrStdCoro_co, ctr_build_string_from_cstring("await"), &ctr_coro_resume);
  ctr_internal_create_func(CtrStdCoro_co, ctr_build_string_from_cstring("run"),
                           &ctr_coro_resume);
  ctr_internal_create_func(
      CtrStdCoro_co, ctr_build_string_from_cstring("yield"), &ctr_coro_yield);
  ctr_internal_create_func(
      CtrStdCoro_co, ctr_build_string_from_cstring("yield:"), &ctr_coro_yield);
  ctr_internal_create_func(CtrStdCoro_co,
                           ctr_build_string_from_cstring("isRunning"),
                           &ctr_coro_isrunning);
  ctr_internal_create_func(
      CtrStdCoro_co, ctr_build_string_from_cstring("state"), &ctr_coro_state);
  CtrStdSymbol = ctr_internal_create_object(CTR_OBJECT_TYPE_OTMISC);
  ctr_set_link_all(CtrStdSymbol, CtrStdObject);
  ctr_internal_create_func(CtrStdSymbol,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_symbol_to_string);
  ctr_internal_create_func(CtrStdSymbol,
                           ctr_build_string_from_cstring("equals:"),
                           &ctr_symbol_equals);
  ctr_internal_create_func(CtrStdSymbol, ctr_build_string_from_cstring("="),
                           &ctr_symbol_equals);
  ctr_internal_create_func(CtrStdSymbol, ctr_build_string_from_cstring("iHash"),
                           &ctr_symbol_ihash);
  ctr_internal_create_func(CtrStdSymbol, ctr_build_string_from_cstring("id"),
                           &ctr_symbol_ihash);
  ctr_internal_create_func(CtrStdSymbol, ctr_build_string_from_cstring("type"),
                           &ctr_symbol_type);
  ctr_internal_create_func(CtrStdSymbol,
                           ctr_build_string_from_cstring("unpack:"),
                           &ctr_symbol_unpack);
  ctr_internal_create_func(CtrStdString,
                           ctr_build_string_from_cstring("toSymbol"),
                           &ctr_string_to_symbol);
  ctr_internal_object_add_property(CtrStdWorld,
                           ctr_build_string_from_cstring("Symbol"),
                           CtrStdSymbol,
                           0);
  ctr_internal_create_func(CtrStdObject,
                           ctr_build_string_from_cstring("inheritFrom:"),
                           &ctr_object_inherit);
  ctr_std_generator = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(ctr_std_generator, CtrStdObject);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("repeat:"),
                           &ctr_generator_make);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("from:to:"),
                           &ctr_generator_make);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("from:to:step:"),
                           &ctr_generator_make);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("elementsOf:"),
                           &ctr_generator_make);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("fmap:"),
                           &ctr_generator_map);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("next"),
                           &ctr_generator_next);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("each:"),
                           &ctr_generator_each);
  ctr_internal_create_func(ctr_std_generator,
                           ctr_build_string_from_cstring("toString"),
                           &ctr_generator_tostr);
  ctr_internal_object_add_property(CtrStdWorld,
                                   ctr_build_string_from_cstring("Generator"),
                                   ctr_std_generator, 0);
}

int mod_accessible(char const *mname) {
  return access(mname, F_OK | X_OK) != -1;
}

void ctr_load_required_native_modules() {

# ifndef CTR_LOAD_BASE_MODS
	return;
# endif
	struct dirent *dp;
  DIR *dfd;
  const static char *dir = CTR_STD_EXTENSION_PATH "/basemods";
  if ((dfd = opendir(dir)) == NULL) {
    perror("Can't open basemods dir");
    return;
  }
  char filename_qfd[1024];

  while ((dp = readdir(dfd)) != NULL) {
		if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
    struct stat stbuf;
    sprintf(filename_qfd, "%s/%s", dir, dp->d_name);
		// printf("%s : (%s)\n", filename_qfd, dp->d_name);
    if (stat(filename_qfd, &stbuf) == -1) {
      printf("Unable to stat file: %s\n", filename_qfd);
      continue;
    }

    if ((stbuf.st_mode & S_IFMT) != S_IFDIR)
      continue;
    else {
      (void)ctr_internal_plugin_find_base(dp->d_name);
    }
  }
}
