#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citron.h"

int ctr_cwlk_replace_refs = 0;
int force_quote = 0;
int ctr_cwlk_msg_level = 0;	//toplevel
int ctr_cwlk_last_msg_level = 0;	//toplevel old
/**
 * CTRWalkerReturn
 *
 * Returns from a block of code.
 */
ctr_object *
ctr_cwlk_return (ctr_tnode * node)
{
  char wasReturn = 0;
  ctr_tlistitem *li;
  ctr_object *e;
  if (!node->nodes)
    {
      printf ("Invalid return expression (Return statement has no structure wtf).\n");
      exit (1);
    }
  li = node->nodes;
  if (!li->node)
    {
      printf ("Invalid return expression 2 (Return statement has no parse node -- %p has no node %p).\n", li, li->node);
      exit (1);
    }
  ctr_current_node_is_return = 1;
  e = ctr_cwlk_expr (li->node, &wasReturn);
  ctr_current_node_is_return = 0;
  return e;
}

/**
 * CTRWalkerMessage
 *
 * Processes a message sending operation.
 */
ctr_object *
ctr_cwlk_message (ctr_tnode * paramNode)
{
  int sticky = 0;
  char wasReturn = 0;
  ctr_object *result;
  ctr_tlistitem *eitem = paramNode->nodes;
  ctr_tnode *receiverNode = eitem->node;
  ctr_tnode *msgnode;
  ctr_tlistitem *li = eitem;
  char *message;
  ctr_tlistitem *argumentList;
  ctr_object *r;
  ctr_object *recipientName = NULL;
  switch (receiverNode->type)
    {
    case CTR_AST_NODE_REFERENCE:
      recipientName = ctr_build_string (receiverNode->value, receiverNode->vlen);
      recipientName->info.sticky = 1;
      if (CtrStdFlow == NULL)
	{
	  ctr_callstack[ctr_callstack_index++] = receiverNode;
	}
      if (receiverNode->modifier == 1 || receiverNode->modifier == 3)
	{
	  r = ctr_find_in_my (recipientName);
	}
      else
	{
	  r = ctr_find (recipientName);
	}
      if (CtrStdFlow == NULL)
	{
	  ctr_callstack_index--;
	}
      if (!r)
	{
	  exit (1);
	}
      break;
    case CTR_AST_NODE_LTRNIL:
      r = ctr_build_nil ();
      break;
    case CTR_AST_NODE_LTRBOOLTRUE:
      r = ctr_build_bool (1);
      break;
    case CTR_AST_NODE_LTRBOOLFALSE:
      r = ctr_build_bool (0);
      break;
    case CTR_AST_NODE_LTRSTRING:
      r = ctr_build_string (receiverNode->value, receiverNode->vlen);
      break;
    case CTR_AST_NODE_LTRNUM:
      r = ctr_build_number_from_string (receiverNode->value, receiverNode->vlen);
      break;
    case CTR_AST_NODE_EMBED:
      if (receiverNode->modifier)
	result = ctr_cwlk_expr (receiverNode->nodes->node, "\0");
      else
	{
	  result = (ctr_object *) receiverNode->nodes->node;
	}
      break;
    case CTR_AST_NODE_LISTCOMP:
      result = ctr_build_listcomp (receiverNode);
      break;
    case CTR_AST_NODE_NATIVEFN:
      result = ctr_internal_create_object (CTR_OBJECT_TYPE_OTNATFUNC);
      result->value.fvalue = (void *) receiverNode->value;
      ctr_set_link_all (result, CtrStdBlock);
      break;
    case CTR_AST_NODE_IMMUTABLE:
    case CTR_AST_NODE_NESTED:
    case CTR_AST_NODE_RAW:
      r = ctr_cwlk_expr (receiverNode, &wasReturn);
      break;
    case CTR_AST_NODE_CODEBLOCK:
      r = ctr_build_block (receiverNode);
      break;
    case CTR_AST_NODE_SYMBOL:
      r = (ctr_object *) (receiverNode->value);
      break;
    default:
      printf ("Cannot send messages to receiver of type: %d \n", receiverNode->type);
      break;
    }
  while (li->next)
    {
      ctr_argument *a;
      ctr_argument *aItem;
      ctr_size l;
      li = li->next;
      msgnode = li->node;
      message = msgnode->value;
      l = msgnode->vlen;
      if (CtrStdFlow == NULL)
	{
	  ctr_callstack[ctr_callstack_index++] = msgnode;
	}
      argumentList = msgnode->nodes;
      a = (ctr_argument *) ctr_heap_allocate (sizeof (ctr_argument));
      aItem = a;
      aItem->object = CtrStdNil;
      if (argumentList)
	{
	  ctr_tnode *node;
	  while (argumentList)
	    {
	      node = argumentList->node;
	      if (!node)
		goto next;
	      aItem->object = ctr_cwlk_expr (node, &wasReturn);
	      /* we always send at least one argument, note that if you want to modify the argumentList, be sure to take this into account */
	      /* there is always an extra empty argument at the end */
	      aItem->next = ctr_heap_allocate (sizeof (ctr_argument));
	      aItem = aItem->next;
	    next:;
	      aItem->object = NULL;
	      argumentList = argumentList->next;
	    }
	}
      sticky = r->info.sticky;
      r->info.sticky = 1;
      result = ctr_send_message (r, message, l, a);
      r->info.sticky = sticky;
      aItem = a;
      if (CtrStdFlow == NULL)
	{
	  ctr_callstack_index--;
	}
      while (aItem->next)
	{
	  a = aItem;
	  aItem = aItem->next;
	  ctr_heap_free (a);
	}
      ctr_heap_free (aItem);
      r = result;
    }
  if (recipientName)
    recipientName->info.sticky = 0;
  return result;
}

/* make sure to allocate enough; size is 16 */
void
ctr_mksrands (char *buf)
{
  static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ1234567890____:;";
  const static int size = 16;
  for (size_t n = 0; n < size; n++)
    {
      int key = rand () % (int) (sizeof charset - 1);
      buf[n] = charset[key];
    }
}

/**
 * CTRWalkerAssignment
 *
 * Processes an assignment operation.
 */
ctr_object *
ctr_cwlk_assignment (ctr_tnode * node)
{
  char wasReturn = 0;
  ctr_tlistitem *assignmentItems = node->nodes;
  ctr_tnode *assignee = assignmentItems->node;
  ctr_tlistitem *valueListItem = assignmentItems->next;
  ctr_tnode *value = valueListItem->node;
  ctr_object *x;
  ctr_object *result;
  char ret;
  if (CtrStdFlow == NULL)
    {
      ctr_callstack[ctr_callstack_index++] = assignee;
    }
  ctr_did_side_effect = 0;
  x = ctr_cwlk_expr (value, &wasReturn);
  if (!x)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("NULL expression");
      return NULL;
    }
  if (assignee->type == CTR_AST_NODE_REFERENCE)
    {
      if (assignee->modifier == 1)
	{
	  result = ctr_assign_value_to_my (ctr_build_string (assignee->value, assignee->vlen), x);
	}
      else if (assignee->modifier == 2)
	{
	  result = ctr_assign_value_to_local (ctr_build_string (assignee->value, assignee->vlen), x);
	}
      else if (assignee->modifier == 3)
	{
	  result = ctr_assign_value (ctr_build_string (assignee->value, assignee->vlen), x);	//Handle lexical scoping
	}
      else if (assignee->modifier == 4)
	{
	  // char* name_s = ctr_heap_allocate(17 * sizeof(char));
	  // name_s[0] = ':'; //make it inaccessable from the system
	  // ctr_mksrands(name_s+1);

	  // ctr_tnode* repl = ctr_heap_allocate(sizeof(*repl));
	  // *repl = *assignee;
	  // assignee->modifier = /* var */2; /* modify for subsequent evaluation */
	  // repl->value = name_s; /* lookup from global */
	  // repl->vlen = 17;

	  // repl->modifier = /* var */2;
	  // valueListItem->node = repl; /* modify for subsequent evaluation */
	  // ctr_heap_free(value);
	  ctr_object *lname = ctr_build_string (assignee->value, assignee->vlen);
	  // ctr_object* name = ctr_build_string(repl->value, repl->vlen);
	  result = ctr_assign_value_to_local (lname, x);	// for this run only
	  // result = ctr_hand_value_to_global (name, x);
	  node->type = CTR_AST_NODE_EMBED;
	  node->modifier = 1;
	  node->nodes = ctr_heap_allocate (sizeof (ctr_tlistitem));
	  node->nodes->node = (ctr_tnode *) x;
	}
      else if (assignee->modifier == 5)
	{
	  if (ctr_did_side_effect)
	    {
	      ctr_heap_free (assignee);
	      *node = *value;
	      result = x;	//we did a boo-boo, fix it
	    }
	  else
	    {
	      char *name_s = ctr_heap_allocate (17 * sizeof (char));
	      name_s[0] = ':';	//make it inaccessable from the system
	      ctr_mksrands (name_s + 1);

	      node->type = CTR_AST_NODE_REFERENCE;
	      node->value = name_s;	/* lookup from global */
	      node->vlen = 17;
	      node->modifier = /* var */ 2;

	      ctr_heap_free (value);
	      ctr_heap_free (assignee);
	      ctr_heap_free (valueListItem);

	      ctr_object *name = ctr_build_string (name_s, 17);
	      result = ctr_hand_value_to_global (name, x);
	    }
	}
      else
	{
	  result = ctr_assign_value (ctr_build_string (assignee->value, assignee->vlen), x);
	}
    }
  else
    {
      int old_replace = ctr_cwlk_replace_refs;
      ctr_cwlk_replace_refs = 1;
      ctr_cwlk_last_msg_level = ctr_cwlk_msg_level;
      ctr_object *y = ctr_cwlk_expr (assignee, &ret);
      ctr_cwlk_replace_refs = old_replace;	//set back in case we didn't reset
      result = ctr_send_message_variadic (x, "unpack:", 7, 2, y, ctr_contexts[ctr_context_id]);	// hand the block the context
    }
  if (CtrStdFlow == NULL)
    {
      ctr_callstack_index--;
    }
  return result;
}

void
execute_if_quote (ctr_tnode * node)
{
  if (node->type == CTR_AST_NODE_EMBED && node->modifier == 0)
    {
      node->modifier = 1;
      node->nodes->node = (ctr_tnode *) ctr_cwlk_expr (node->nodes->node, "");
      return;
    }
  switch (node->type)
    {
    case CTR_AST_NODE_CODEBLOCK:
      execute_if_quote (node->nodes->next->node);
      return;
    case CTR_AST_NODE_EXPRMESSAGE:
    case CTR_AST_NODE_EXPRASSIGNMENT:
      execute_if_quote (node->nodes->node);
      execute_if_quote (node->nodes->next->node);
      return;
    case CTR_AST_NODE_BINMESSAGE:
    case CTR_AST_NODE_RAW:
    case CTR_AST_NODE_NESTED:
      execute_if_quote (node->nodes->node);
      return;
    case CTR_AST_NODE_IMMUTABLE:
    case CTR_AST_NODE_PROGRAM:
      node = node->nodes->node;
      /* Fallthrough */
    case CTR_AST_NODE_KWMESSAGE:
    case CTR_AST_NODE_INSTRLIST:
      for (ctr_tlistitem * instr = node->nodes; instr; instr = instr->next)
	execute_if_quote (instr->node);
      return;
      // TODO X: handle listcomp
    default:
      break;
    }
}

/**
 * CTRWalkerExpression
 *
 * Processes an expression.
 */
ctr_object *
ctr_cwlk_expr (ctr_tnode * node, char *wasReturn)
{
  char c;
  if (!node)
    {
      CtrStdFlow = ctr_build_string_from_cstring ("Encounered null parse node");
      return NULL;
    }
  ctr_object *result;
  switch (node->type)
    {
    case CTR_AST_NODE_LTRSTRING:
      result = ctr_build_string (node->value, node->vlen);
      break;
    case CTR_AST_NODE_LTRBOOLTRUE:
      result = ctr_build_bool (1);
      break;
    case CTR_AST_NODE_LTRBOOLFALSE:
      result = ctr_build_bool (0);
      break;
    case CTR_AST_NODE_LTRNIL:
      result = ctr_build_nil ();
      break;
    case CTR_AST_NODE_LTRNUM:
      result = ctr_build_number_from_string (node->value, node->vlen);
      break;
    case CTR_AST_NODE_CODEBLOCK:
      result = ctr_build_block (node);
      break;
    case CTR_AST_NODE_EMBED:
      if (node->modifier)
	result = (ctr_object *) node->nodes->node;
      else
	{
	  result = ctr_cwlk_expr (node->nodes->node, "");
	}
      break;
    case CTR_AST_NODE_REFERENCE:
      if ((ctr_cwlk_replace_refs /*&& ctr_cwlk_msg_level <= ctr_cwlk_last_msg_level */ ) || force_quote)
	{
	  result = ctr_get_or_create_symbol_table_entry (node->value, node->vlen);
	  break;
	}
      if (CtrStdFlow == NULL)
	{
	  ctr_callstack[ctr_callstack_index++] = node;
	}
      if (node->modifier == 1 || node->modifier == 3)
	{
	  result = ctr_find_in_my (ctr_build_string (node->value, node->vlen));
	}
      else
	result = ctr_find (ctr_build_string (node->value, node->vlen));
      if (CtrStdFlow == NULL)
	{
	  ctr_callstack_index--;
	}
      break;
    case CTR_AST_NODE_EXPRMESSAGE:
      result = ctr_cwlk_message (node);
      break;
    case CTR_AST_NODE_EXPRASSIGNMENT:
      result = ctr_cwlk_assignment (node);
      break;
    case CTR_AST_NODE_IMMUTABLE:
      result = ctr_build_immutable (node);
      break;
    case CTR_AST_NODE_SYMBOL:
      result = (ctr_object *) (node->value);
      break;
    case CTR_AST_NODE_RAW:
      {
	int quote = 0;
	switch (node->modifier)
	  {
	  case 1:
	    execute_if_quote (node->nodes->node);
	    result = ctr_ast_from_node (node->nodes->node);
	    break;
	  case 2:
	    result = ctr_build_immutable (node->nodes->node);
	    break;
	  case -1:
	    quote = 1;
	    /* Fallthrough */
	  case 0:
	    {
	      char ret;
	      int oldquote = force_quote, refs = ctr_cwlk_replace_refs;
	      force_quote = quote;
	      ctr_cwlk_last_msg_level = ctr_cwlk_msg_level;
	      ctr_cwlk_replace_refs = 0;
	      result = ctr_cwlk_expr (node->nodes->node, &ret);
	      ctr_cwlk_replace_refs = refs;
	      force_quote = oldquote;	//set back in case we didn't reset
	      if (ctr_ast_is_splice (result))
		result = ctr_ast_splice (result);
	      break;
	    }
	  }
      }
      break;
    case CTR_AST_NODE_RETURNFROMBLOCK:
      result = ctr_cwlk_return (node);
      *wasReturn = 1;
      break;
    case CTR_AST_NODE_NESTED:
      result = ctr_cwlk_expr (node->nodes->node, wasReturn);
      break;
    case CTR_AST_NODE_LISTCOMP:
      result = ctr_build_listcomp (node);
      break;
    case CTR_AST_NODE_NATIVEFN:
      result = ctr_internal_create_object (CTR_OBJECT_TYPE_OTNATFUNC);
      result->value.fvalue = (void *) node->value;
      ctr_set_link_all (result, CtrStdBlock);
      break;
    case CTR_AST_NODE_ENDOFPROGRAM:
      if (CtrStdFlow && CtrStdFlow != CtrStdExit && ctr_cwlk_subprogram == 0)
	{
	  printf ("Uncaught error has occurred.\n");
	  if (CtrStdFlow->info.type == CTR_OBJECT_TYPE_OTSTRING)
	    {
	      fwrite (CtrStdFlow->value.svalue->value, sizeof (char), CtrStdFlow->value.svalue->vlen, stdout);
	      printf ("\n");
	    }
	  ctr_print_stack_trace ();
	}
      result = ctr_build_nil ();
      break;
    default:
      printf ("Runtime Error. Invalid parse node: %d %s \n", node->type, node->value);
      exit (1);
      break;
    }
  return result;
}

/**
 * CTRWalkerRunBlock
 *
 * Processes the execution of a block of code.
 */

ctr_object *
ctr_cwlk_run (ctr_tnode * program)
{
  ctr_cwlk_msg_level++;
  ctr_object *result = NULL;
  char wasReturn = 0;
  ctr_tlistitem *li;
  li = program->nodes;
  while (li)
    {
      ctr_tnode *node = li->node;
      if (!li->node)
	{
	  // printf("Missing parse node\n");
	  // exit(1);
	  CtrStdFlow = ctr_build_string_from_cstring ("Missing Parse Node");
	  return NULL;
	}
      wasReturn = 0;
      result = ctr_cwlk_expr (node, &wasReturn);
      if (ctr_internal_next_return)
	{
	  wasReturn = 1;
	  ctr_internal_next_return = 0;
	  break;
	}
      if (wasReturn)
	{
	  break;
	}
      /* Perform garbage collection cycle */
      if (((ctr_gc_mode & 1) && ctr_gc_alloc > (ctr_gc_memlimit * 0.8)) || ctr_gc_mode & 4)
	{
	  printf ("GC : %d bytes\n", ctr_gc_alloc);
	  ctr_gc_internal_collect ();	//collect on limit mode
	  printf ("GC : %d bytes\n", ctr_gc_alloc);
	}
      if (!li->next)
	break;
      li = li->next;
    }
  if (wasReturn == 0 && !program->lexical)	//let the last value be returned on BLOCKMAP
    result = NULL;
  ctr_cwlk_msg_level--;
  return result;
}
