#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citron.h"

#pragma optimize 0

#define malloc ctr_heap_allocate

const char **ctr_ast_node_names[] = {
	"exprassignment",
	"exprmessage",
	"unamessage",
	"binmessage",
	"kwmessage",
	"ltrstring",
	"reference",
	"ltrnum",
	"codeblock",
	"returnfromblock",
	"immutable",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"(unknown)",
	"paramlist",
	"instrlist",
	"",
	"endofprogram",
	"nested",
	"ltrbooltrue",
	"ltrboolfalse",
	"ltrnil",
	"program"
};

struct environment {
	int length;
	unsigned int index;
	char *binding;
	ctr_tnode *value;
	int deterministic;
	int indeterministic_level;
	struct environment *next;
	struct environment *parent;
};
typedef struct environment environment;

environment senv = {.parent = NULL,.binding = "",.length = 0,.index = 0 };

environment *env = &senv;
int env_index = 0;
int env_context_index = 0;

void ctr_ccomp_free_env(environment * ctx)
{
	environment *env_ = ctx->next;
	while (env_) {
		//free(ctx);
		env_ = env_->next;
	}
}

void ctr_ccomp_env_pprint_popped(environment * env, int indent)
{
	if (!env)
		return;
	environment *ctx = env;
	while (ctx != (environment *) 0 && ctx) {
		printf("%*c-%.*s\n", indent, ' ', ctx->length, ctx->binding);
		ctx = ctx->next;
	}
}

environment *ctr_ccomp_env_maybe_get_ref(volatile environment * env,
					 ctr_tnode * node)
{
	volatile environment *ctx = env;
	while (ctx && node) {
		printf("-- %d\n", ctx->index);
		if (ctx->length == node->vlen) {
			printf("checking '%.*s' against '%.*s'\n", node->vlen,
			       node->value, ctx->length, ctx->binding);
			if (node->vlen == ctx->length
			    && strncmp(node->value, ctx->binding,
				       node->vlen) == 0)
				return ctx;
			printf("failed check against '%.*s', resuming\n",
			       ctx->length, ctx->binding);
		}
		environment *maybe_in_parent =
		    ctr_ccomp_env_maybe_get_ref(ctx->parent, node);
		if (maybe_in_parent)
			return maybe_in_parent;
		ctx = ctx->next;
	}
	return NULL;
}

void ctr_ccomp_env_update_new_context()
{
	env_context_index = env_index++;
	environment *ctx = malloc(sizeof(environment));
	ctx->index = 0;
	ctx->next = NULL;
	ctx->parent = env;
	env = ctx;
}

environment *ctr_ccomp_env_update_pop_context()
{
	if (!env->parent)
		return env;
	env_index = env_context_index;
	environment *ctx = env;
	env = env->parent;
	// ctr_ccomp_env_pprint_popped(ctx, 1);
	return ctx;
}

void ctr_ccomp_env_update_block(environment * env, ctr_tnode * node)
{
	//TODO
}

void ctr_ccomp_env_update_assign(environment ** env, ctr_tlistitem * nodes)
{
	ctr_tnode *name = nodes->node;
	ctr_tnode *value = nodes->next->node;
	if (value->type == CTR_AST_NODE_REFERENCE) {
		printf("Querying environment for binding %.*s\n", value->vlen,
		       value->value);
		ctr_tnode *alt = ctr_ccomp_env_maybe_get_ref(*env, value);
		if (alt) {
			printf("environment has %p for binding %.*s\n", alt,
			       value->vlen, value->value);
			value = alt;
		} else
			printf
			    ("No binding found for %.*s, assuming runtime generated\n",
			     value->vlen, value->value);
	}
	environment *new_env = malloc(sizeof(environment));
	new_env->binding = name->value;
	new_env->length = name->vlen;
	printf("Updating environment with binding %.*s = %p\n", name->vlen,
	       name->value, value);
	new_env->value = malloc(sizeof(value));
	memcpy(new_env->value, value, sizeof(value));
	new_env->indeterministic_level =
	    ctr_ccomp_node_indeterministic_level(value);
	new_env->deterministic = new_env->indeterministic_level < 2 ? 1 : 0;
	printf("\nreached conclusion %s (level %d)\n",
	       new_env->deterministic ? "deterministic" : "indeterministic",
	       new_env->indeterministic_level);
	new_env->next = *env;
	new_env->index = (*env)->index + 1;
	*env = new_env;
}

void
ctr_ccomp_env_update_assign_charstar(environment ** env, char *name, int length,
				     ctr_tnode * value)
{
	environment *new_env = malloc(sizeof(environment));
	new_env->binding = malloc(sizeof(char) * length + 1);
	memcpy(new_env->binding, name, length);
	new_env->length = length;
	printf("Updating environment with binding %.*s = %p\n", name, length,
	       value);
	new_env->value = malloc(sizeof(value));
	memcpy(new_env->value, value, sizeof(value));
	new_env->indeterministic_level =
	    ctr_ccomp_node_indeterministic_level(value);
	new_env->deterministic = new_env->indeterministic_level < 2 ? 1 : 0;
	printf("\nreached conclusion %s (level %d)\n",
	       new_env->deterministic ? "deterministic" : "indeterministic",
	       new_env->indeterministic_level);
	new_env->next = *env;
	new_env->index = (*env)->index + 1;
	*env = new_env;
}

/* level 0 : value directly available at compile time
 * level 1 : value recursively available at compile time
 * level 2 : value possibilities available at compile time
 * level 3 : value not available at compile time
 */
int ctr_ccomp_node_indeterministic_level(ctr_tnode * node);
int ctr_ccomp_rec_nodes_indeterministic_level(ctr_tlistitem * nodes)
{
	int max_level = 0;
	ctr_tlistitem *node = nodes;
	while (node) {
		max_level =
		    fmax(max_level,
			 ctr_ccomp_node_indeterministic_level(node->node));
		node = node->next;
	}
	return max_level;
}

int lastcci = -1;
int ctr_ccomp_node_indeterministic_level(ctr_tnode * node)
{
	if (node == NULL)
		return 0;	//we already have the value
	if (lastcci > -1)
		printf("CHECK = %d\n", lastcci);
	printf("CHECK : looking for deterministicness of %p\n", node);
	if (node->type >= 51 && node->type <= 84 && node->nodes) {
		printf("%s " "{\n",
		       ctr_ast_node_names[node->type -
					  CTR_AST_NODE_EXPRASSIGNMENT]);
		//ctr_internal_debug_tree(node, 2);
		printf(" }\n");
	}
	switch (node->type) {
	case CTR_AST_NODE_LTRNIL:
	case CTR_AST_NODE_LTRNUM:
	case CTR_AST_NODE_LTRSTRING:
	case CTR_AST_NODE_LTRBOOLTRUE:
	case CTR_AST_NODE_LTRBOOLFALSE:
	case CTR_AST_NODE_ENDOFPROGRAM:
		return lastcci = 0;
	case CTR_AST_NODE_PARAMLIST:
		{		//if we've been handed this, we have started parsing a block
			ctr_tlistitem *parameterList = node->nodes;
			if (!parameterList)
				return 0;
			ctr_tnode *param = parameterList->node;
			if (param == NULL) {
				return lastcci = 0;
			}
			while ((param = parameterList->node) != NULL) {
				int vararg = *(param->value) == '*';
				ctr_ccomp_env_update_assign_charstar(&env,
								     param->
								     value +
								     vararg,
								     param->
								     vlen -
								     vararg,
								     NULL);
				parameterList = parameterList->next;
				if (parameterList == NULL)
					break;
			}
			return lastcci = 0;
		}
	case CTR_AST_NODE_NESTED:
		return lastcci =
		    ctr_ccomp_node_indeterministic_level(node->nodes->node);
	case CTR_AST_NODE_REFERENCE:
		{
			environment *fy =
			    ctr_ccomp_env_maybe_get_ref(env, node);
			if (fy) {
				printf
				    ("environment had an entry for %.*s : %p (level %d)\n",
				     fy->length, fy->binding, fy->value,
				     fy->indeterministic_level);
				return lastcci = fy->indeterministic_level;
			} else
				return lastcci = 3;	//maybe a runtime generated binding
		}
	case CTR_AST_NODE_IMMUTABLE:
		{
			int max_level = 0;
			ctr_tlistitem *nn = node->nodes;
			while (nn) {
				max_level =
				    fmax(max_level,
					 ctr_ccomp_rec_nodes_indeterministic_level
					 (nn));
				nn = nn->next;
			}
			return lastcci = max_level;
		}
	case CTR_AST_NODE_UNAMESSAGE:
		{
			return lastcci = 1;	//we know this message inside and out
		}
	case CTR_AST_NODE_BINMESSAGE:
		return lastcci = ctr_ccomp_node_indeterministic_level(node->nodes->node);	//argument
	case CTR_AST_NODE_KWMESSAGE:
		return lastcci = ctr_ccomp_rec_nodes_indeterministic_level(node->nodes);	//all the arguments
	case CTR_AST_NODE_EXPRMESSAGE:
		{
			int rec = ctr_ccomp_node_indeterministic_level(node->nodes->node);	//receiver
			int msgs = ctr_ccomp_node_indeterministic_level(node->nodes->next->node);	//messages
			if (rec < 2 && msgs < 2) {
				//we know the value one way or the other
				return lastcci = 1;	//we can recursively figure out the result of this expression
			} else
				return lastcci = fmax(rec, msgs);	//whichever is worse
		}
	case CTR_AST_NODE_RETURNFROMBLOCK:
		return lastcci =
		    ctr_ccomp_node_indeterministic_level(node->nodes->node);
	case CTR_AST_NODE_EXPRASSIGNMENT:
		ctr_ccomp_env_update_assign(&env, node->nodes);
		return lastcci = ctr_ccomp_node_indeterministic_level(node->nodes->next->node);	//assignee
	case CTR_AST_NODE_PROGRAM:
		return lastcci =
		    ctr_ccomp_rec_nodes_indeterministic_level(node->nodes);
	case CTR_AST_NODE_CODEBLOCK:
		{
			ctr_ccomp_env_update_new_context();
			ctr_ccomp_node_indeterministic_level(node->nodes->node);	//paramlist is level 0, but we need to handle it
			int max = ctr_ccomp_rec_nodes_indeterministic_level(node->nodes->next);	//get the worst of our instructions
			ctr_ccomp_env_update_pop_context();
			return lastcci = max;
		}
	case CTR_AST_NODE_INSTRLIST:
		return lastcci =
		    ctr_ccomp_rec_nodes_indeterministic_level(node->nodes);
	case 1:
		return lastcci = 0;
	default:
		{
			printf("we have no rule for type %d = %s\n", node->type,
			       ctr_ast_node_names[node->type -
						  CTR_AST_NODE_EXPRASSIGNMENT]);
			return lastcci = 3;	//we have no clue
		}
	}
}

ctr_tnode *ctr_ccomp_as_literal(ctr_tnode * node);

ctr_tlistitem *ctr_ccomp_nodes_as_literal(ctr_tlistitem * li)
{
	ctr_tlistitem *oli = li;
	while (li) {
		printf(" => %p <=> %s\n", li->node,
		       ctr_ast_node_names[li->node->type -
					  CTR_AST_NODE_EXPRASSIGNMENT]);
		li->node = ctr_ccomp_as_literal(li->node);
		li = li->next;
	}
	return oli;
}

ctr_tnode *ctr_ccomp_as_literal(ctr_tnode * node)
{
	printf(" → %p >< %s\n", node,
	       ctr_ast_node_names[node->type - CTR_AST_NODE_EXPRASSIGNMENT]);
	switch (node->type) {
	case CTR_AST_NODE_LTRNIL:
	case CTR_AST_NODE_LTRNUM:
	case CTR_AST_NODE_LTRSTRING:
	case CTR_AST_NODE_LTRBOOLTRUE:
	case CTR_AST_NODE_LTRBOOLFALSE:
	case CTR_AST_NODE_ENDOFPROGRAM:
		return node;
	case CTR_AST_NODE_IMMUTABLE:
		node->nodes = ctr_ccomp_nodes_as_literal(node->nodes);
		return node;
	case CTR_AST_NODE_NESTED:
		return ctr_ccomp_as_literal(node->nodes->node);
	case CTR_AST_NODE_REFERENCE:
		{
			printf
			    ("Got a reference to turn into a literal namely %.*s\n",
			     node->vlen, node->value);
			environment *_e =
			    ctr_ccomp_env_maybe_get_ref(env, node);
			if (_e)
				return _e->value;
			else
				return node;
			break;
		}
	case CTR_AST_NODE_EXPRASSIGNMENT:
		{
			ctr_tnode *onode = malloc(sizeof(ctr_tnode));
			onode->nodes = malloc(sizeof(ctr_tlistitem));
			onode->nodes->node = node->nodes->node;
			onode->nodes->next = malloc(sizeof(ctr_tlistitem));
			onode->nodes->next->node =
			    ctr_ccomp_as_literal(node->nodes->next->node);
			return onode;
		}
	default:
		return node;
	}
}

volatile ctr_tnode *ctr_ccomp_optimize_node_inplace(volatile ctr_tnode ** node);
ctr_tlistitem *ctr_ccomp_optimize_nodes_inplace(ctr_tlistitem * nodes)
{
	ctr_tlistitem *node = nodes;
	while (node) {
		node->node = ctr_ccomp_optimize_node_inplace(&(node->node));
		node = node->next;
	}
	return nodes;
}

volatile ctr_tnode *ctr_ccomp_optimize_node_inplace(volatile ctr_tnode volatile
						    **node)
{
	printf("Got a(n) %s (%p), optimization start\n",
	       ctr_ast_node_names[(*node)->type - CTR_AST_NODE_EXPRASSIGNMENT],
	       node);
	switch ((*node)->type) {
	case CTR_AST_NODE_NESTED:
		{
			ctr_tlistitem *nodes = (*node)->nodes;
			return ctr_ccomp_optimize_node_inplace(&(nodes->node));	//get rid of the nesting
		}
	case CTR_AST_NODE_IMMUTABLE:
	case CTR_AST_NODE_KWMESSAGE:
	case CTR_AST_NODE_UNAMESSAGE:
	case CTR_AST_NODE_INSTRLIST:
		{
			int deter = 3;
			if (deter =
			    (ctr_ccomp_node_indeterministic_level(node)) < 2) {
				printf("proceeding optimization of %p\n", node);
				(*node)->nodes =
				    ctr_ccomp_nodes_as_literal((*node)->nodes);
			}
			printf("DETERMINISTIC STATUS OF %p = level %d\n", node,
			       deter);
			return *node;
		}
	case CTR_AST_NODE_EXPRASSIGNMENT:
		{
			//node->nodes->node : no need to optimize a reference
			printf("Type = %s\n",
			       ctr_ast_node_names[(*node)->nodes->next->node->
						  type -
						  CTR_AST_NODE_EXPRASSIGNMENT]);
			(*node)->nodes->next->node =
			    ctr_ccomp_optimize_node_inplace(&
							    ((*node)->nodes->
							     next->node));
			//update the environment
			ctr_ccomp_env_update_assign(&env, (*node)->nodes);
			return *node;
		}
	case CTR_AST_NODE_CODEBLOCK:
		{
			//node->nodes->node : no need to optimize the params
			ctr_ccomp_env_update_new_context();
			(*node)->nodes->next =
			    ctr_ccomp_optimize_nodes_inplace((*node)->nodes->
							     next);
			ctr_ccomp_env_update_block
			    (ctr_ccomp_env_update_pop_context(),
			     (*node)->nodes->next->node);
			return *node;
		}
	case CTR_AST_NODE_EXPRMESSAGE:
		{
			int deter = 3;
			if ((deter =
			     ctr_ccomp_node_indeterministic_level((*node)->
								  nodes->
								  node)) < 2) {
				printf("proceeding optimization of %p\n", node);
				(*node)->nodes->node =
				    ctr_ccomp_as_literal((*node)->nodes->node);
				(*node)->nodes->next->node->nodes =
				    ctr_ccomp_nodes_as_literal((*node)->nodes->
							       next->node->
							       nodes);
			}
			printf("DETERMINISTIC STATUS OF %p = level %d\n", node,
			       deter);
			return *node;
		}
	case CTR_AST_NODE_PROGRAM:
		ctr_ccomp_optimize_nodes_inplace((*node)->nodes);
	default:
		return *node;
	}
}

/***********************************Code Gen***********************************/

uint64_t ctr_ccomp_subprogram;
ctr_object *ctr_ccomp_run(ctr_tnode * program);
ctr_object *ctr_ccomp_expr(ctr_tnode * node, char *wasReturn);

int ctr_ccomp_print_series(ctr_tlistitem * item, char space)
{
	char i;
	char spacer[3];
	if (space != '\0') {
		spacer[0] = space;
		spacer[1] = ' ';
		spacer[2] = '\0';
	} else {
		spacer[0] = ' ';
		spacer[1] = '\0';
	}
	while (item) {
		ctr_ccomp_expr(item->node, &i);
		if (item->next) {
			printf("%s", spacer);
			item = item->next;
		} else
			break;
	}
	return 0;
}

int ctr_ccomp_print_series_raw(ctr_tlistitem * item, char space)
{
	char i;
	char spacer[3];
	if (space != '\0') {
		spacer[0] = space;
		spacer[1] = ' ';
		spacer[2] = '\0';
	} else {
		spacer[0] = ' ';
		spacer[1] = '\0';
	}
	while (item) {
		printf("%.*s", item->node->vlen, item->node->value);
		if (item->next) {
			printf("%s", spacer);
			item = item->next;
		} else
			break;
	}
	return 0;
}

int
ctr_ccomp_print_series_raw_transform(ctr_tlistitem * item, char space, char *tr,
				     char *to)
{
	char i;
	char spacer[3];
	int trlen = strlen(tr);
	if (space != '\0') {
		spacer[0] = space;
		spacer[1] = ' ';
		spacer[2] = '\0';
	} else {
		spacer[0] = ' ';
		spacer[1] = '\0';
	}
	while (item) {
		if (item->node->vlen >= trlen
		    && strncmp(item->node->value, tr, trlen) == 0)
			printf("%s%.*s", to, (item->node->vlen - trlen),
			       item->node->value + trlen);
		else
			printf("%.*s", item->node->vlen, item->node->value);
		if (item->next) {
			printf("%s", spacer);
			item = item->next;
		} else
			break;
	}
	return 0;
}

/**
 * CTRCompilerReturn
 *
 * Returns from a block of code.
 */
ctr_object *ctr_ccomp_return(ctr_tnode * node)
{
	char wasReturn = 0;
	ctr_tlistitem *li;
	ctr_object *e;
	if (!node->nodes) {
		printf("Invalid return expression.\n");
		exit(1);
	}
	li = node->nodes;
	if (!li->node) {
		printf("Invalid return expression 2.\n");
		exit(1);
	}
	printf("return ");
	e = ctr_ccomp_expr(li->node, &wasReturn);
	return e;
}

int ctr_ccomp_str_replace(char *str, char o, char r)
{
	char *ix = str;
	int n = 0;
	while ((ix = strchr(ix, o)) != NULL) {
		*ix++ = r;
		n++;
	}
	return n;
}

/**
 * CTRCompilerMessage
 *
 * Processes a message sending operation.
 */
ctr_object *ctr_ccomp_message(ctr_tnode * paramNode)
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
	volatile ctr_object volatile *r;
	ctr_object *recipientName = NULL;
	switch (receiverNode->type) {
	case CTR_AST_NODE_REFERENCE:
		recipientName =
		    ctr_build_string(receiverNode->value, receiverNode->vlen);
		recipientName->info.sticky = 1;
		int can_be_self = 1;
		if (CtrStdFlow == NULL) {
			ctr_callstack[ctr_callstack_index++] = receiverNode;
		}
		if (receiverNode->modifier == 1 || receiverNode->modifier == 3) {
			printf("this.");
			can_be_self = 0;
			r = ctr_find_in_my(recipientName);
		} else {
			r = ctr_find(recipientName);
		}
		if (can_be_self
		    &&
		    (strncmp
		     (receiverNode->value, "me",
		      fmin(2, receiverNode->vlen)) == 0)) {
			printf("this");
		} else {
			printf("%.*s", receiverNode->vlen, receiverNode->value);
		}
		if (CtrStdFlow == NULL) {
			ctr_callstack_index--;
		}
		// if (!r) {
		//     exit(1);
		// }
		break;
	case CTR_AST_NODE_LTRNIL:
		r = ctr_build_nil();
		printf("null");
		break;
	case CTR_AST_NODE_LTRBOOLTRUE:
		r = ctr_build_bool(1);
		printf("true");
		break;
	case CTR_AST_NODE_LTRBOOLFALSE:
		r = ctr_build_bool(0);
		printf("false");
		break;
	case CTR_AST_NODE_LTRSTRING:
		r = ctr_build_string(receiverNode->value, receiverNode->vlen);
		printf("\'%.*s\'", receiverNode->vlen, receiverNode->value);
		break;
	case CTR_AST_NODE_LTRNUM:
		r = ctr_build_number_from_string(receiverNode->value,
						 receiverNode->vlen);
		printf("%.*s", receiverNode->vlen, receiverNode->value);
		break;
	case CTR_AST_NODE_IMMUTABLE:
		printf("[");
		ctr_ccomp_print_series(receiverNode->nodes->node->nodes, ',');
		printf("]");
		break;
	case CTR_AST_NODE_NESTED:
		printf("(");
		r = ctr_ccomp_expr(receiverNode, &wasReturn);
		printf(")");
		break;
	case CTR_AST_NODE_CODEBLOCK:
		ctr_ccomp_print_block(receiverNode);
		r = ctr_build_block(receiverNode);
		break;
	default:
		printf("Cannot send messages to receiver of type: %d \n",
		       receiverNode->type);
		break;
	}
	int dd = 0;
	while (li->next) {
		ctr_argument *a;
		ctr_argument *aItem;
		ctr_size l;
		li = li->next;
		msgnode = li->node;
		message = msgnode->value;
		l = msgnode->vlen;
		if (CtrStdFlow == NULL) {
			ctr_callstack[ctr_callstack_index++] = msgnode;
		}
		argumentList = msgnode->nodes;
		a = (ctr_argument *) ctr_heap_allocate(sizeof(ctr_argument));
		aItem = a;
		aItem->object = CtrStdNil;
		char *str = strndup(message, l);
		printf("['%.*s'](", l, str);
		free(str);
		if (argumentList) {
			ctr_tnode *node;
			node = argumentList->node;
			while (1) {
				ctr_object *o =
				    ctr_ccomp_expr(node, &wasReturn);
				aItem->object = o;
				/* we always send at least one argument, note that if you want to modify the argumentList, be sure to take this into account */
				/* there is always an extra empty argument at the end */
				aItem->next =
				    (ctr_argument *)
				    ctr_heap_allocate(sizeof(ctr_argument));
				aItem = aItem->next;
				aItem->object = NULL;
				if (!argumentList->next)
					break;
				printf(", ");
				argumentList = argumentList->next;
				node = argumentList->node;
			}
		}
		printf(")");
		// sticky = r->info.sticky;
		// r->info.sticky = 1;
		result = ctr_send_message(r, message, l, a);
		// r->info.sticky = sticky;
		aItem = a;
		if (CtrStdFlow == NULL) {
			ctr_callstack_index--;
		}
		while (aItem->next) {
			a = aItem;
			aItem = aItem->next;
			ctr_heap_free(a);
		}
		ctr_heap_free(aItem);
		r = result;
	}
	if (recipientName)
		recipientName->info.sticky = 0;
	return result;
}

/**
 * CTRCompilerAssignment
 *
 * Processes an assignment operation.
 */
ctr_object *ctr_ccomp_assignment(ctr_tnode * node)
{
	char wasReturn = 0;
	ctr_tlistitem *assignmentItems = node->nodes;
	ctr_tnode *assignee = assignmentItems->node;
	ctr_tlistitem *valueListItem = assignmentItems->next;
	ctr_tnode *value = valueListItem->node;
	ctr_object *x;
	ctr_object *result;
	if (CtrStdFlow == NULL) {
		ctr_callstack[ctr_callstack_index++] = assignee;
	}
	if (assignee->modifier == 1) {
		printf("(this.%.*s) = ", assignee->vlen, assignee->value);
		x = ctr_ccomp_expr(value, &wasReturn);
		result =
		    ctr_assign_value_to_my(ctr_build_string
					   (assignee->value, assignee->vlen),
					   x);
	} else if (assignee->modifier == 2) {
		printf("%.*s = ", assignee->vlen, assignee->value);
		x = ctr_ccomp_expr(value, &wasReturn);
		result =
		    ctr_assign_value_to_local(ctr_build_string
					      (assignee->value, assignee->vlen),
					      x);
	} else if (assignee->modifier == 3) {
		printf("(this.%.*s) = ", assignee->vlen, assignee->value);
		x = ctr_ccomp_expr(value, &wasReturn);
		result = ctr_assign_value(ctr_build_string(assignee->value, assignee->vlen), x);	//Handle lexical scoping
	} else {
		printf("%.*s = ", assignee->vlen, assignee->value);
		x = ctr_ccomp_expr(value, &wasReturn);
		result =
		    ctr_assign_value(ctr_build_string
				     (assignee->value, assignee->vlen), x);
	}
	if (CtrStdFlow == NULL) {
		ctr_callstack_index--;
	}
	return result;
}

void ctr_ccomp_print_block(ctr_tnode * node)
{
	ctr_tlistitem *item = node->nodes->node->nodes;
	printf("(function(");
	ctr_ccomp_print_series_raw_transform(item, ',', "*", "var");
	printf(") {");
	item = node->nodes->next->node->nodes;
	ctr_ccomp_print_series(item, ';');
	printf("})");
}

/**
 * CTRCompilerExpression
 *
 * Processes an expression.
 */
ctr_object *ctr_ccomp_expr(ctr_tnode * node, char *wasReturn)
{
	ctr_object *result;
	switch (node->type) {
	case CTR_AST_NODE_LTRSTRING:
		result = ctr_build_string(node->value, node->vlen);
		printf("\"%.*s\"", node->vlen, node->value);
		break;
	case CTR_AST_NODE_LTRBOOLTRUE:
		result = ctr_build_bool(1);
		printf("true");
		break;
	case CTR_AST_NODE_LTRBOOLFALSE:
		result = ctr_build_bool(0);
		printf("false");
		break;
	case CTR_AST_NODE_LTRNIL:
		result = ctr_build_nil();
		printf("NULL");
		break;
	case CTR_AST_NODE_LTRNUM:
		result = ctr_build_number_from_string(node->value, node->vlen);
		printf("%.*s", node->vlen, node->value);
		break;
	case CTR_AST_NODE_CODEBLOCK:
		result = ctr_build_block(node);
		ctr_ccomp_print_block(node);
		break;
	case CTR_AST_NODE_REFERENCE:
		if (CtrStdFlow == NULL) {
			ctr_callstack[ctr_callstack_index++] = node;
		}
		if (node->modifier == 1 || node->modifier == 3) {
			result =
			    ctr_find_in_my(ctr_build_string
					   (node->value, node->vlen));
			printf("(this.%.*s)", node->vlen, node->value);
		} else {
			result =
			    ctr_find(ctr_build_string(node->value, node->vlen));
			printf("%.*s", node->vlen, node->value);
		}
		if (CtrStdFlow == NULL) {
			ctr_callstack_index--;
		}
		break;
	case CTR_AST_NODE_EXPRMESSAGE:
		result = ctr_ccomp_message(node);
		break;
	case CTR_AST_NODE_EXPRASSIGNMENT:
		result = ctr_ccomp_assignment(node);
		break;
	case CTR_AST_NODE_IMMUTABLE:
		printf("[");
		ctr_ccomp_print_series(node->nodes->node->nodes, ',');
		printf("]");
		result = ctr_build_immutable(node);
		break;
	case CTR_AST_NODE_RETURNFROMBLOCK:
		result = ctr_ccomp_return(node);
		*wasReturn = 1;
		break;
	case CTR_AST_NODE_NESTED:
		printf("(");
		result = ctr_ccomp_expr(node->nodes->node, wasReturn);
		printf(")");
		break;
	case CTR_AST_NODE_ENDOFPROGRAM:
		//if (CtrStdFlow && CtrStdFlow != CtrStdExit && ctr_ccomp_subprogram == 0) {
		//    printf("Uncaught error has occurred.\n");
		//    if (CtrStdFlow->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		//        fwrite(CtrStdFlow->value.svalue->value, sizeof(char), CtrStdFlow->value.svalue->vlen, stdout);
		//        printf("\n");
		//    }
		//    ctr_print_stack_trace();
		//}
		result = ctr_build_nil();
		break;
	default:
		fprintf(stderr, "Runtime Error. Invalid parse node: %d %s \n",
			node->type, node->value);
		exit(1);
		break;
	}
	return result;
}

/**
 * CTRCompilerRunBlock
 *
 * Processes the execution of a block of code.
 */

ctr_object *ctr_ccomp_run(ctr_tnode * program)
{
	ctr_object *result = NULL;
	char wasReturn = 0;
	ctr_tlistitem *li;
	li = program->nodes;
	while (li) {
		ctr_tnode volatile *node = li->node;
		if (!li->node) {
			printf("Missing parse node\n");
			exit(1);
		}
		wasReturn = 0;
		result = ctr_ccomp_expr(node, &wasReturn);
		puts("");
		if (ctr_internal_next_return) {
			wasReturn = 1;
			ctr_internal_next_return = 0;
			break;
		}
		if (wasReturn) {
			break;
		}
		//if ( ( ( ctr_gc_mode & 1 ) && ctr_gc_alloc > ( ctr_gc_memlimit * 0.8 ) ) || ctr_gc_mode & 4 ) {
		//  ctr_gc_internal_collect(); //collect on limit mode
		//}
		if (!li->next)
			break;
		li = li->next;
	}
	if (wasReturn == 0)
		result = NULL;
	return result;
}
