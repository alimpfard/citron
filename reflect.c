#include <stdio.h>
#include <stdlib.h>
#include "citron.h"


ctr_object* ctr_reflect_new(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = myself;
    return instance;
}

/**
 * Reflect addGlobalVariable: [name:String]
 *
 * adds a variable named <name> to the global context
 */
ctr_object* ctr_reflect_add_glob (ctr_object* myself, ctr_argument* argumentList) {
    CTR_ENSURE_TYPE_STRING(argumentList->object);
    if(ctr_internal_object_find_property(CtrStdWorld, argumentList->object, 0)) return myself; //We already have it
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);
    instance->link = CtrStdNil;
    instance->info.sticky = 0;
    ctr_internal_object_add_property(CtrStdWorld, ctr_internal_cast2string(argumentList->object), instance, 0);
    return instance;
}

/**
 * Reflect addLocalVariable: [name:String]
 *
 * adds a variable named <name> to the Local context
 */
ctr_object* ctr_reflect_add_local (ctr_object* myself, ctr_argument* argumentList) {
    CTR_ENSURE_TYPE_STRING(argumentList->object);
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = CtrStdObject;
    ctr_internal_object_add_property(ctr_contexts[ctr_context_id], ctr_internal_cast2string(argumentList->object), instance, CTR_CATEGORY_PUBLIC_PROPERTY);
    return CtrStdNil;
}

/**
 * Reflect addPrivateVariable: [name:String]
 *
 * adds a variable named <name> to this context
 */
ctr_object* ctr_reflect_add_my (ctr_object* myself, ctr_argument* argumentList) {
    CTR_ENSURE_TYPE_STRING(argumentList->object);
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = CtrStdObject;
    ctr_internal_object_add_property(ctr_contexts[ctr_context_id], ctr_internal_cast2string(argumentList->object), instance, CTR_CATEGORY_PRIVATE_PROPERTY);
    return CtrStdNil;
}

/**
 * Reflect set: [name:String] to: [Object]
 *
 * dynamically sets a binding
 */
ctr_object* ctr_reflect_set_to(ctr_object* myself, ctr_argument* argumentList) {
    CTR_ENSURE_TYPE_STRING(argumentList->object);
    ctr_internal_object_set_property(CtrStdWorld, ctr_internal_cast2string(argumentList->object), argumentList->next->object, 0);
    return CtrStdNil;
}

/**
 * Reflect getContext
 *
 * returns all the context keys
 */
ctr_object* ctr_reflect_dump_context(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* instance = ctr_array_new(CtrStdArray, NULL);
    ctr_object* child;
    ctr_object* props;
    ctr_object* meths;
    int i = ctr_context_id;
    while(i>-1) {
        //printf("%d\n", i);
        props = ctr_array_new(CtrStdArray, NULL);
        meths = ctr_array_new(CtrStdArray, NULL);
        child = ctr_array_new(CtrStdArray, NULL); //Props; Methods
        int p = ctr_contexts[i]->properties->size - 1;
        struct ctr_mapitem* head = ctr_contexts[i]->properties->head;
        while(p>-1) {
            //printf("p:%d:%d :: ", i, p);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = head->key;
            args->next = NULL;
            //printf("%s\n", head->key->value.svalue->value);
            ctr_array_push(props, args);
            ctr_heap_free(args);
            head = head->next;
            p--;
        }
        int m = ctr_contexts[i]->methods->size - 1;
        head = ctr_contexts[i]->properties->head;
        while(m>-1) {
            //printf("m:%d:%d :: ", i, m);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = head->key;
            args->next = NULL;
            //printf("%s\n", head->key->value.svalue->value);
            ctr_array_push(meths, args);
            ctr_heap_free(args);
            head = head->next;
            m--;
        }
        ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
        args->object = props;
        args->next = NULL;
        ctr_array_push(child, args);
        args->object = meths;
        args->next = NULL;
        ctr_array_push(child, args);
        args->object = child;
        args->next = NULL;
        ctr_array_push(instance, args);
        ctr_heap_free(args);
        i--;
    }
    return instance;
}

/**
 * Reflect getMethodsOf: [Object]
 *
 * returns all the method names of object
 */
ctr_object* ctr_reflect_dump_context_spec(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* of = argumentList->object;
    ctr_object* meths = ctr_array_new(CtrStdArray, NULL);
    ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
    while(of->link) {
      int m = of->methods->size - 1;
      struct ctr_mapitem* head;
      head = of->methods->head;
      while(m>-1) {
          //printf("m:%d:%d :: ", i, m);
          args->object = head->key;


            args->next = NULL;
          //printf("%s\n", head->key->value.svalue->value);
          ctr_array_push(meths, args);
          head = head->next;
          m--;
      }
      if (of->link) of = of->link;
      else break;
    }
    ctr_heap_free(args);
    return meths;
}
int ctr_internal_has_responder(ctr_object* of, ctr_object* meth) {
  while(of->link) {
    int m = of->methods->size - 1;
    struct ctr_mapitem* head;
    head = of->methods->head;
    while(m>-1) {
        if (ctr_internal_object_is_equal(head->key, meth)) goto return_true;
        head = head->next;
        m--;
    }
    if (of->link) of = of->link;
    else break;
  }
  return 0;
  return_true:;
  return 1;
}
int ctr_internal_has_own_responder(ctr_object* of, ctr_object* meth) {
  int m = of->methods->size - 1;
  struct ctr_mapitem* head;
  head = of->methods->head;
  while(m>-1) {
      if (ctr_internal_object_is_equal(head->key, meth)) goto return_true;
      head = head->next;
      m--;
  }
  return 0;
  return_true:;
  return 1;
}
ctr_object* ctr_reflect_has_own_responder(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* of = argumentList->object;
  ctr_object* meth = argumentList->next->object;
  return ctr_build_bool(ctr_internal_has_own_responder(of, meth));
}

/**
 * Reflect getPropertiesOf: [Object]
 *
 * returns all the property names of object
 */
ctr_object* ctr_reflect_dump_context_spec_prop(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* of = argumentList->object;
    ctr_object* props = ctr_array_new(CtrStdArray, NULL);
    int p = of->properties->size - 1;
    struct ctr_mapitem* head;
    head = of->properties->head;
    while(p>-1) {
        //printf("m:%d:%d :: ", i, m);
        ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
        args->object = head->key;
        args->next = NULL;
        //printf("%s\n", head->key->value.svalue->value);
        ctr_array_push(props, args);
        ctr_heap_free(args);
        head = head->next;
        p--;
    }
    return props;
}

/**
 * [Reflect] getObject: [s:String]
 *
 * looks for the object `s` in the current context or any of the contexts beneath
 **/
ctr_object* ctr_reflect_find_obj(ctr_object* myself, ctr_argument* argumentList) {
    CTR_ENSURE_TYPE_STRING(argumentList->object);
    return ctr_find(ctr_internal_cast2string(argumentList->object));
}

/**
 * [Reflect] objectExists: [Object]
 *
 * returns whether the object exists
 */
ctr_object* ctr_reflect_extst_obj(ctr_object* myself, ctr_argument* argumentList) {
    CTR_ENSURE_TYPE_STRING(argumentList->object);
    int i = ctr_context_id;
    ctr_object* foundObject = NULL;
    ctr_object* key = ctr_internal_cast2string(argumentList->object);
    while((i>-1 && foundObject == NULL)) {
        ctr_object* context = ctr_contexts[i];
        foundObject = ctr_internal_object_find_property(context, key, 0);
        i--;
    }
    if (foundObject == NULL) return ctr_build_bool(0);
    else return ctr_build_bool(1);
}

ctr_object* ctr_reflect_cb_ac(ctr_object* myself, ctr_argument* argumentList) {
  CTR_ENSURE_TYPE_BLOCK(argumentList->object);
  ctr_tnode* obj = argumentList->object->value.block;
  if(obj == NULL) return CtrStdNil;
  ctr_tlistitem* parameterList = obj->nodes->node->nodes;
  ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = CtrStdNil;
  ctr_object* arglist = ctr_send_message(CtrStdArray, "new", 3, args);
  if (parameterList == NULL) {
    ctr_heap_free(args);
    return arglist;
  }
  ctr_tnode* param = parameterList->node;
  if(param == NULL) {
    ctr_heap_free(args);
    return arglist;
  }
  while((param = parameterList->node) != NULL) {
    args->object = (parameterList->next == NULL && *(param->value) == '*') ? ctr_build_string(param->value+1, param->vlen-1) : ctr_build_string(param->value, param->vlen);
    ctr_send_message(arglist, "push:", 5, args);
    parameterList = parameterList->next;
    if(parameterList == NULL) break;
  }
  ctr_heap_free(args);
  return arglist;
}

ctr_object* ctr_reflect_cb_ac_(ctr_object* myself, ctr_argument* argumentList) {
  CTR_ENSURE_TYPE_BLOCK(argumentList->object);
  ctr_tnode* obj = argumentList->object->value.block;
  if(obj == NULL) return CtrStdNil;
  ctr_tlistitem* parameterList = obj->nodes->node->nodes;
  ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = CtrStdNil;
  ctr_object* arglist = ctr_send_message(CtrStdArray, "new", 3, args);
  if (parameterList == NULL) {
    ctr_heap_free(args);
    return arglist;
  }
  ctr_tnode* param = parameterList->node;
  if(param == NULL) {
    ctr_heap_free(args);
    return arglist;
  }
  while((param = parameterList->node) != NULL) {
    args->object = (parameterList->next == NULL && *(param->value) == '*') ? ctr_build_string_from_cstring("*Binding") : ctr_build_string_from_cstring("Binding");
    ctr_send_message(arglist, "push:", 5, args);
    parameterList = parameterList->next;
    if(parameterList == NULL) break;
  }
  ctr_heap_free(args);
  return arglist;
}
#ifdef UNPARSE
ctr_object* ctr_reflect_get_nodes(ctr_tnode* ti, ctr_object* ns) {
  ctr_tlistitem* li;
  ctr_tnode* t;
  li = ti->nodes;
  if(!li) return;
  t = li->node;
  ctr_object* retval;
  while(1) {
      switch (t->type) {
          case CTR_AST_NODE_REFERENCE: {
            switch (t->modifier) {
              case 1: retval = ctr_build_string_from_cstring("my "); break;
              case 2: retval = ctr_build_string_from_cstring("var "); break;
              case 3: retval = ctr_build_string_from_cstring("const "); break;
              default: retval = ctr_build_empty_string(); break;
            }
            ctr_invoke_variadic(retval, &ctr_string_append, 1, ctr_build_string_from_cstring(t->value));
            break;
          }
          case CTR_AST_NODE_EXPRASSIGNMENT: {
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_object* assignee = ctr_reflect_get_nodes(t->nodes->node, ns);
            ctr_object* assigned = ctr_reflect_get_nodes(t->nodes->next->node, ns);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = ctr_build_string_from_cstring("assign");
            ctr_array_push(retval, args);
            args->object = assignee;
            ctr_array_push(retval, args);
            args->object = assigned;
            ctr_array_push(retval, args);
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_EXPRMESSAGE: {
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_object* receiver = ctr_reflect_get_nodes(t->nodes->node, ns);
            ctr_object* message = ctr_reflect_get_nodes(t->nodes->next->node, ns);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = ctr_build_string_from_cstring("message");
            ctr_array_push(retval, args);
            args->object = receiver;
            ctr_array_push(retval, args);
            args->object = message;
            ctr_array_push(retval, args);
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_LTRSTRING:
          case CTR_AST_NODE_LTRNUM:
          case CTR_AST_NODE_LTRBOOLTRUE:
          case CTR_AST_NODE_LTRBOOLFALSE:
          case CTR_AST_NODE_LTRNIL:
          case CTR_AST_NODE_UNAMESSAGE: {
            retval = ctr_build_string(t->value, t->vlen);
            break;
          }
          case CTR_AST_NODE_BINMESSAGE: {
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_object* msgargs = ctr_array_new(CtrStdArray, NULL);

            ctr_object* name = ctr_build_string(t->value, t->vlen);
            ctr_object* arg  = ctr_reflect_get_nodes(t->nodes->node, ns);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = name;
            ctr_array_push(retval, args);
            args->object = arg;
            ctr_array_push(msgargs, args);
            args->object = msgargs;
            ctr_array_push(retval, args);
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_KWMESSAGE: { //TODO: Unparse all argument nodes
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_object* msgargs = ctr_array_new(CtrStdArray, NULL);
            ctr_object* name = ctr_build_string(t->value, t->vlen);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = name;
            ctr_array_push(retval, args);
            args->object = msgargs;
            ctr_array_push(retval, args);
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_CODEBLOCK: {
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            args->object = ctr_build_block(t);
            ctr_object* receiver = ctr_reflect_cb_ac(CtrStdReflect, args);
            ctr_object* message = ctr_reflect_get_nodes(t->nodes->next->node, ns);
            args->object = ctr_build_string_from_cstring("block");
            ctr_array_push(retval, args);
            args->object = receiver;
            ctr_array_push(retval, args);
            args->object = message;
            ctr_array_push(retval, args);
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_PARAMLIST: {
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_tlistitem* parameterList = t->nodes;
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            if (parameterList == NULL) {
              ctr_heap_free(args);
              break;
            }
            ctr_tnode* param = parameterList->node;
            if(param == NULL) {
              ctr_heap_free(args);
              break;
            }
            while((param = parameterList->node) != NULL) {
              args->object = ctr_build_string(param->value, param->vlen);
              ctr_send_message(retval, "push:", 5, args);
              parameterList = parameterList->next;
              if(parameterList == NULL) break;
            }
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_INSTRLIST: {
            retval = ctr_array_new(CtrStdArray, NULL);
            ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
            ctr_tlistitem* instrli = t->nodes;
            while(1) {
              if(!instrli->node) break;
              args->object = ctr_reflect_get_nodes(instrli->node, ns);
              ctr_array_push(retval, args);
              if(!instrli->next) break;
              instrli = instrli->next;
            }
            ctr_heap_free(args);
            break;
          }
          case CTR_AST_NODE_IMMUTABLE:
          case CTR_AST_NODE_NESTED:
          case CTR_AST_NODE_RETURNFROMBLOCK:
          case CTR_AST_NODE_ENDOFPROGRAM:
          case CTR_AST_NODE_PROGRAM: {
            perror("Unimpl");
            break;
          }
      }
      if (!li->next) break;
      li = li->next;
      t = li->node;
  }
  return retval;
}

ctr_object* ctr_reflect_dump_function(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_reflect_get_nodes(argumentList->object->value.block, argumentList->object);
}

#endif

ctr_object* ctr_reflect_unparse(ctr_object* arr, int type) {
  int args = ctr_invoke_variadic(arr, &ctr_array_count, 0)->value.nvalue;
  if (args < 1) return ctr_build_empty_string();
  switch (type) {
    case CTR_AST_NODE_EXPRMESSAGE:
    case CTR_AST_NODE_KWMESSAGE:
    case CTR_AST_NODE_UNAMESSAGE:
    case CTR_AST_NODE_BINMESSAGE:
    {
      ctr_object* fmt = ctr_invoke_variadic(arr, &ctr_array_get, 1, ctr_build_number_from_float(0)); // receiver
      if (args > 1) {
        ctr_invoke_variadic(fmt, &ctr_string_append, 1, ctr_build_string_from_cstring(" message: '"));
        ctr_object* msg = ctr_invoke_variadic(arr, &ctr_array_get, 1, ctr_build_number_from_float(1));
        ctr_invoke_variadic(fmt, &ctr_string_append, 1, ctr_invoke_variadic(msg, &ctr_array_get, 1, ctr_build_number_from_float(0))); // message name
        ctr_invoke_variadic(fmt, &ctr_string_append, 1, ctr_build_string_from_cstring("' arguments: ("));
        ctr_invoke_variadic(fmt, &ctr_string_append, 1, ctr_invoke_variadic(msg, &ctr_array_get, 1, ctr_build_number_from_float(1))); // args
        ctr_invoke_variadic(fmt, &ctr_string_append, 1, ctr_build_string_from_cstring(")"));
      }
      return fmt;
    }
    //
    case CTR_AST_NODE_LTRNIL:
    case CTR_AST_NODE_LTRNUM:
    case CTR_AST_NODE_LTRSTRING:
    case CTR_AST_NODE_LTRBOOLTRUE:
    case CTR_AST_NODE_LTRBOOLFALSE:
    case CTR_AST_NODE_REFERENCE:
    return ctr_invoke_variadic(arr, &ctr_array_get, 1, ctr_build_number_from_float(0));
    //
    case CTR_AST_NODE_EXPRASSIGNMENT:
    return ctr_invoke_variadic(arr, &ctr_array_join, 1, ctr_build_string_from_cstring(" is "));
    //
    case CTR_AST_NODE_NESTED:
    case CTR_AST_NODE_RETURNFROMBLOCK:
    return ctr_invoke_variadic(arr, &ctr_array_join, 1, ctr_build_empty_string());
    //
    default:
    printf("%d\n", type);
    return ctr_internal_cast2string(arr);
  }
}

ctr_object* ctr_reflect_serialize_expr_(ctr_tnode* expr_node, ctr_argument* burrowed, int i);
ctr_object* ctr_reflect_serialize_expr(ctr_tnode* expr_node, ctr_argument* b) {
  return ctr_reflect_serialize_expr_(expr_node, b, 0);
}
ctr_object* ctr_reflect_serialize_expr_(ctr_tnode* expr_node, ctr_argument* burrowed, int i) {
  ctr_object* ser = ctr_array_new(CtrStdArray, NULL);
  ctr_tlistitem* nodes = expr_node->nodes;
  ctr_tnode* node;
  while (nodes && (node = nodes->node) != NULL) {
    if (nodes == NULL) break;
    if (node->value) {
      burrowed->object = ctr_build_string(node->value, node->vlen);
      if (node->type == CTR_AST_NODE_EXPRMESSAGE || node->type == CTR_AST_NODE_KWMESSAGE || node->type == CTR_AST_NODE_BINMESSAGE || node->type == CTR_AST_NODE_UNAMESSAGE) {
        ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
        ctr_object* ps = ctr_array_new(CtrStdArray, NULL);
        ctr_array_push(arr, burrowed);
        burrowed->object = ctr_reflect_serialize_expr_(node, burrowed, 1);
        ctr_array_push(ps, burrowed);
        burrowed->object = ps;
        ctr_array_push(arr, burrowed);
        arr->value.avalue->immutable=1;
        burrowed->object = arr;
      }
    }
    else             burrowed->object = ctr_reflect_serialize_expr_(node, burrowed, 1);
    ctr_array_push(ser, burrowed);
    nodes = nodes->next;
  }
  ser->value.avalue->immutable=1;
  return ctr_reflect_unparse(ser, expr_node->type);
}

ctr_object* ctr_reflect_cb_ic(ctr_object* myself, ctr_argument* argumentList) {
  ctr_tnode* obj = argumentList->object->value.block;
  if(obj == NULL) return CtrStdNil;
  ctr_tlistitem* parameterList = obj->nodes->next->node->nodes;
  ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
  args->object = CtrStdNil;
  ctr_object* arglist = ctr_array_new(CtrStdArray, NULL);
  if (parameterList == NULL) {
    ctr_heap_free(args);
    return arglist;
  }
  ctr_tnode* param = parameterList->node;
  if(param == NULL) {
    ctr_heap_free(args);
    return arglist;
  }
  while((param = parameterList->node) != NULL) {
    if (param->value) args->object = ctr_build_string(param->value, param->vlen);
    else              args->object = ctr_reflect_serialize_expr(param, args);
    ctr_send_message(arglist, "push:", 5, args);
    parameterList = parameterList->next;
    if(parameterList == NULL) break;
  }
  ctr_heap_free(args);
  return arglist;
}

/**
 * [Reflect] addArgumentTo: [Block] named: [s:String]
 *
 * adds a new argument `s` to the block
 */
ctr_object* ctr_reflect_cb_add_param(ctr_object* myself, ctr_argument* argumentList) {
  CTR_ENSURE_TYPE_BLOCK(argumentList->object);
  ctr_object* obj = argumentList->object;
  ctr_tlistitem* parameterList = obj->value.block->nodes->node->nodes;
  if (parameterList == NULL) {
    parameterList = ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
    parameterList->node = ctr_heap_allocate_tracked(sizeof(ctr_tnode));
    char* name = ctr_heap_allocate_cstring(argumentList->next->object);
    parameterList->node->value = ctr_heap_allocate_tracked(sizeof(name));
    memcpy(parameterList->node->value, name, strlen(name));
    ctr_heap_free(name);
    parameterList->node->vlen = strlen(name);
    obj->value.block->nodes->node->nodes = parameterList;
    return myself;
  }
  while (parameterList->next != NULL) parameterList = parameterList->next;

  parameterList->next = ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
  parameterList->next = ctr_heap_allocate_tracked(sizeof(ctr_tlistitem));
  parameterList->next->node = ctr_heap_allocate_tracked(sizeof(ctr_tnode));
  parameterList = parameterList->next;
  char* name = ctr_heap_allocate_cstring(argumentList->next->object);
  parameterList->node->value = ctr_heap_allocate_tracked(sizeof(name));
  int len = memcpy(parameterList->node->value, name, strlen(name));
  ctr_heap_free(name);
  parameterList->node->vlen = len;
  return myself;
}

/**
 * [Reflect] copyBlock: [b:Block]
 *
 * copies block b to a new context
 */
ctr_object* ctr_reflect_fn_copy(ctr_object* myself, ctr_argument* argumentList) {
  CTR_ENSURE_TYPE_BLOCK(argumentList->object);
  if(argumentList->object->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
    CtrStdFlow = ctr_build_string_from_cstring("parameter was of an invalid type.");
    return CtrStdNil;
  }
  ctr_object* newblk = ctr_build_block(argumentList->object->value.block);
  ctr_object* name = ctr_build_string_from_cstring("inheritance");
  ctr_object* prop = ctr_internal_object_find_property(argumentList->object, name, CTR_CATEGORY_PRIVATE_PROPERTY);
  if(prop != NULL) ctr_internal_object_add_property(newblk, name, prop, CTR_CATEGORY_PRIVATE_PROPERTY);
  newblk->info.sticky = argumentList->object->info.sticky;
  return newblk;
}

/**
 * [Reflect] newSharedObject
 *
 * makes a memmapped object. (Automatic destructor exists)
 **/
ctr_object* ctr_reflect_share_memory(ctr_object* myself, ctr_argument* argumentList) {
  // CTR_ENSURE_TYPE_STRING(argumentList->object);
  ctr_object* shared = ctr_internal_create_mapped_object(CTR_OBJECT_TYPE_OTOBJECT, 1);
  shared->link = CtrStdObject;
  return shared;
}

/**
 * [Reflect] link: [o:Object] to: [p:Object]
 *
 * casts o to object type p
 **/
ctr_object* ctr_reflect_link_to(ctr_object* myself, ctr_argument* argumentList) {
  argumentList->object->link = argumentList->next->object;
  return argumentList->object;
}

/**
 * [Reflect] objectExists: [o:String]
 *
 * checks whether the object named o exists
 **/
ctr_object* ctr_reflect_find_obj_ex(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* maybefound = ctr_reflect_find_obj(myself, argumentList);
  CtrStdFlow = NULL;
  if(maybefound == CtrStdNil) return ctr_build_bool(0);
  return ctr_build_bool(1);
}

int ctr_reflect_is_linked_to_(ctr_argument* argumentList) {
  if (argumentList->object->link == NULL) return 0;
  ctr_object* link = argumentList->object->link;
  argumentList->object = link;
  return (link == argumentList->next->object) || ctr_reflect_is_linked_to_(argumentList);
}

/**
 * [Reflect] isObject: [o:Object] linkedTo: [p:Object]
 *
 * checks whether p is anywhere in o's link chain
 **/
ctr_object* ctr_reflect_is_linked_to(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(ctr_reflect_is_linked_to_(argumentList));
}

/**
 * [Reflect] isObject: [o:Object] childOf: [p:Object]
 *
 * returns whether p is the parent of o
 **/
ctr_object* ctr_reflect_child_of(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_bool(argumentList->object->link == argumentList->next->object);
}

/**
 * [Reflect] generateLinkTree: [o:Object]
 *
 * generates a linear tree, representing the chain of inheritance for object o
 **/
ctr_object* ctr_reflect_generate_inheritance_tree(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* parent = argumentList->object;
  ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
  ctr_argument* newarg = ctr_heap_allocate(sizeof(ctr_argument));
  while(parent != NULL) {
    newarg->object = (parent->lexical_name==NULL?parent:parent->lexical_name);
    ctr_array_push(arr, newarg);
    if (parent == parent->link) goto cleanup;
    parent = parent->link;
  }
  cleanup:
  ctr_heap_free(newarg);
  return arr;
}

ctr_object* ctr_reflect_type_descriptor_print(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* type_descriptor = ctr_build_string_from_cstring("Nil");
  if (argumentList->object == NULL) return type_descriptor;
  //ctr_object* object = ctr_is_primitive(argumentList->object)?argumentList->object:ctr_reflect_describe_type(myself, argumentList);
  ctr_object* object = argumentList->object;
  switch (object->info.type) {
    case CTR_OBJECT_TYPE_OTNIL:
        type_descriptor = ctr_build_string_from_cstring("Nil");
        break;
    case CTR_OBJECT_TYPE_OTBOOL:
        type_descriptor = ctr_build_string_from_cstring("Boolean");
        break;
    case CTR_OBJECT_TYPE_OTNUMBER:
        type_descriptor = ctr_build_string_from_cstring("Number");
        break;
    case CTR_OBJECT_TYPE_OTSTRING:
        type_descriptor = ctr_build_string_from_cstring("String");
        break;
    case CTR_OBJECT_TYPE_OTBLOCK:
        type_descriptor = ctr_invoke_variadic(myself, &ctr_reflect_type_descriptor_print, 1, ctr_invoke_variadic(myself, &ctr_reflect_cb_ac_, 1, object));
        type_descriptor = ctr_invoke_variadic(type_descriptor, &ctr_string_concat, 1, ctr_build_string_from_cstring(" -> Object"));
        break;
    case CTR_OBJECT_TYPE_OTNATFUNC:
        type_descriptor = ctr_build_string_from_cstring("[NativeFunction]");
        break;

    case CTR_OBJECT_TYPE_OTOBJECT:
        type_descriptor = ctr_send_message(object, "type", 4, NULL);
        //TODO: Implement
        break;
    case CTR_OBJECT_TYPE_OTARRAY:
        //List ds, generate a list of content types
        type_descriptor = ctr_array_new(CtrStdArray, NULL);
        int i = 0;
        int c = ctr_array_count(object, NULL)->value.nvalue;
        ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
        for(;i<c;i++) {
          arg->object = ctr_build_number_from_float(i);
          arg->object = ctr_array_get(object, arg);
          arg->object = ctr_reflect_type_descriptor_print(myself, arg);
          ctr_array_push(type_descriptor, arg);
        }
        ctr_heap_free(arg);
        type_descriptor->value.avalue->immutable = 1;
        type_descriptor = ctr_internal_cast2string(type_descriptor);
        break;
    case CTR_OBJECT_TYPE_OTMISC:
    case CTR_OBJECT_TYPE_OTEX:
        //Is not data, just return Nil
        break;
  }

  return type_descriptor;
}

ctr_object* ctr_reflect_get_primitive_link(ctr_object* object) {
  ctr_object* parent = object;
  while(parent!=NULL) {
    if (ctr_is_primitive(parent)) break;
      parent = parent->link;
  }
  return parent;
}

/**
 * [Reflect] primitiveLinkOf: [o:Object]
 *
 * gets the first primitive object associated with o in its link chain
 **/
ctr_object* ctr_reflect_get_primitive(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_reflect_get_primitive_link(argumentList->object);
}

/**
 * [Reflect] typeOf: [Object]
 *
 * returns a type constructor suited for the object
 **/
ctr_object* ctr_reflect_describe_type(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* object = argumentList->object;
  ctr_object* link = object->link;
  ctr_object* type_descriptor = CtrStdNil;
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));

  switch (object->info.type) {
    case CTR_OBJECT_TYPE_OTNIL:
    case CTR_OBJECT_TYPE_OTBOOL:
    case CTR_OBJECT_TYPE_OTNUMBER:
    case CTR_OBJECT_TYPE_OTSTRING:
    case CTR_OBJECT_TYPE_OTNATFUNC:
        //No Data structure, just primitive, return primitive link
        type_descriptor = ctr_reflect_get_primitive_link(object);
        arg->object = type_descriptor;
        type_descriptor = ctr_reflect_type_descriptor_print(myself, arg);

        break;

    case CTR_OBJECT_TYPE_OTBLOCK:
      type_descriptor = ctr_reflect_cb_ac_(myself, argumentList);
      type_descriptor->value.avalue->immutable = 1;
      break;

    case CTR_OBJECT_TYPE_OTOBJECT:
        //Map ds, generate a map of content types.
        ctr_reflect_map_type_descriptor = ctr_string_eval(
          ctr_build_string_from_cstring("{:kv ^[(kv @ 0), (Reflect typeOf: kv @ 1)].}"),
          NULL
        );
        arg->object = ctr_reflect_map_type_descriptor;
        type_descriptor = ctr_map_kvmap(object, arg);
        break;
    case CTR_OBJECT_TYPE_OTARRAY:
        //List ds, generate a list of content types
        type_descriptor = ctr_array_new(CtrStdArray, NULL);
        int i = 0;
        int c = ctr_array_count(object, NULL)->value.nvalue;
        ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
        for(;i<c;i++) {
          arg->object = ctr_build_number_from_float(i);
          arg->object = ctr_array_get(object, arg);
          arg->object = ctr_reflect_describe_type(myself, arg);
          ctr_array_push(type_descriptor, arg);
        }
        type_descriptor->value.avalue->immutable = 1;
        break;
    case CTR_OBJECT_TYPE_OTMISC:
    case CTR_OBJECT_TYPE_OTEX:
        //Is not data, just return Nil
        break;
  }

  ctr_heap_free(arg);
  return type_descriptor;
}
ctr_object* ctr_reflect_describe_value(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* object = argumentList->object;
  ctr_object* link = object->link;
  ctr_object* type_descriptor = CtrStdNil;
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));

  switch (object->info.type) {
    case CTR_OBJECT_TYPE_OTOBJECT:
        //Map ds, generate a list of content values.
        ctr_reflect_map_type_descriptor = ctr_string_eval(
          ctr_build_string_from_cstring("{:kv ^[(kv @ 0), (kv @ 1)].}"),
          NULL
        );
        arg->object = ctr_reflect_map_type_descriptor;
        type_descriptor = ctr_map_kvlist(object, arg);
        break;
    default:
        type_descriptor = object;
        break;
  }
  ctr_heap_free(arg);
  return type_descriptor;
}

ctr_object* ctr_reflect_describe_type_pretty(ctr_object* myself, ctr_argument* argumentList) {
  if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTSTRING) {
    ctr_object* desc = ctr_build_string_from_cstring("String(length=");
    argumentList->object = ctr_internal_cast2string(ctr_build_number_from_float(argumentList->object->value.svalue->vlen));
    ctr_string_append(desc, argumentList);
    argumentList->object = ctr_build_string_from_cstring(")");
    ctr_string_append(desc, argumentList);
    return desc;
  }
  else return ctr_reflect_describe_type(myself, argumentList);
}

int ctr_reflect_is_valid_ctor(ctr_object* candidate) {
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  arg->next = ctr_heap_allocate(sizeof(ctr_argument));
  arg->object = candidate;
  arg->next = CtrStdReflect_cons;
  if (!ctr_reflect_child_of(CtrStdReflect, arg)->value.bvalue) return 0;
  return 1; //TODO: Implement actual method
}

int ctr_reflect_check_bind_valid(ctr_object* from, ctr_object* to) {
  ctr_argument* argumentList = ctr_heap_allocate(sizeof(ctr_argument));
  argumentList->object = to;
  ctr_object* to_type = ctr_reflect_describe_type(CtrStdReflect, argumentList);
  argumentList->object = from;
  ctr_object* from_type = ctr_reflect_describe_type(CtrStdReflect, argumentList);
  if (!(ctr_internal_object_is_constructible_(from_type, to_type, to->info.raw))) {
    CtrStdFlow = ctr_build_string_from_cstring("Cannot bind object of type ");
    argumentList->object = from;
    from_type = ctr_reflect_describe_type_pretty(CtrStdReflect, argumentList);
    argumentList->object = ctr_internal_cast2string(from_type);
    ctr_string_append(CtrStdFlow, argumentList);
    argumentList->object = ctr_build_string_from_cstring(" to object of type ");
    ctr_string_append(CtrStdFlow, argumentList);
    argumentList->object = ctr_build_string_from_cstring("String");
    argumentList->next = ctr_heap_allocate(sizeof(ctr_argument));
    argumentList->next->object = ctr_build_string_from_cstring("Binding");

    argumentList->object = ctr_string_replace_with(ctr_internal_cast2string(to_type), argumentList);
    ctr_heap_free(argumentList->next);
    ctr_string_append(CtrStdFlow, argumentList);
    argumentList->object = ctr_build_string_from_cstring(".");
    ctr_string_append(CtrStdFlow, argumentList);
    ctr_heap_free(argumentList);
    return 0;
  }
  ctr_heap_free(argumentList);
  return 1;
}

ctr_object* ctr_reflect_bind(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* from = argumentList->object;
  ctr_object* to   = argumentList->next->object;
  //if(!ctr_reflect_check_bind_valid(from, to)) {
  //  return CtrStdNil;
  //}
  ctr_argument* arg2 = ctr_heap_allocate(sizeof(ctr_argument));
  arg2->object = to;
  ctr_send_message(from, "unpack:", 7, arg2);
  ctr_heap_free(arg2);
  return myself;
}

ctr_object* ctr_reflect_cons_of(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* ins = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ins->link = myself;
  ctr_internal_object_set_property(ins, ctr_build_string_from_cstring("value"), argumentList->object, 0);
  return ins;
}

ctr_object* ctr_reflect_try_serialize_block(ctr_object* myself, ctr_argument* argumentList) {
  CTR_ENSURE_TYPE_BLOCK(argumentList->object);
  ctr_object* obj = argumentList->object;
  // ctr_tnode* block = argumentList->object->value.block;
  argumentList->object = ctr_reflect_cb_ac(myself, argumentList);
  ctr_object* mp = ctr_map_new(CtrStdMap, NULL);
  ctr_object* params = argumentList->object; //Serialized the args
  ctr_invoke_variadic(mp, &ctr_map_put, 2, params, ctr_build_string_from_cstring("arguments"));
  argumentList->object = obj;
  ctr_object* instrs = ctr_reflect_cb_ic(myself, argumentList);
  ctr_invoke_variadic(mp, &ctr_map_put, 2, instrs, ctr_build_string_from_cstring("instructions"));
  return mp;
}

/**
 * [Reflect] parentOf: [o:Object]
 *
 * gets the first immediate parent of o.
 **/
ctr_object* ctr_reflect_get_first_link(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* link = argumentList->object->link;
  if(link)
    return link;
  else {
    CtrStdFlow = ctr_build_string_from_cstring("Attempt to get parent of native object (NULL deref)");
    return CtrStdNil;
  }
}

/**
 * [Reflect] getResponder: [r:String] ofObject: [o:Object]
 *
 * gets the method r from object o, AS IS
 * The returned block will not contain any references to o.
 * So a context must be supplied. (Takes a 'self' argument first)
 **/
ctr_object* ctr_reflect_get_responder(ctr_object* myself, ctr_argument* argumentList) {
  if(argumentList == NULL || argumentList->next == NULL) CTR_ERR("Argument cannot be NULL.");
  CTR_ENSURE_TYPE_STRING(argumentList->object);
  ctr_object* name  = argumentList->object;
  ctr_object* obj = argumentList->next->object;
  ctr_object* methodObject = ctr_get_responder(obj, name->value.svalue->value, name->value.svalue->vlen);
  if(methodObject&&methodObject->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
    ctr_object* mn = ctr_build_string_from_cstring("{:self:*args ^self message: '");
    ctr_invoke_variadic(mn, &ctr_string_append, 1, name);
    ctr_invoke_variadic(mn, &ctr_string_append, 1, ctr_build_string_from_cstring("' arguments: args.}"));
    // ctr_invoke_variadic(CtrStdConsole, &ctr_console_write, 1, mn);
    ctr_object* ret = ctr_string_eval(mn, NULL);
    return ret;
  }
  if(methodObject) {
    ctr_object* meth = ctr_string_eval(ctr_build_string_from_cstring("{:self:*args ^Reflect run: my method forObject: self arguments: args.}"), NULL);
    ctr_internal_object_add_property(meth, ctr_build_string_from_cstring("method"), methodObject, 0);
    return meth;
  }
  return CtrStdNil;
}

ctr_object* ctr_reflect_object_delegate_get_responder(ctr_object* itself, ctr_argument* argumentList) {
  ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
  arg->object = argumentList->object;
  arg->next = ctr_heap_allocate(sizeof(ctr_argument));
  arg->next->object = itself;
  ctr_object* ret = ctr_reflect_get_responder(CtrStdReflect, arg);
  ctr_heap_free(arg->next);
  ctr_heap_free(arg);
  return ret;
}

/**
 * [Reflect] run: [Block] forObject: [object:Object] arguments: [Array]
 *
 * runs a block with its 'me'/'my' set to object
 **/
ctr_object* ctr_reflect_run_for_object(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object *blk, *me, *argl;
  CTR_ENSURE_TYPE_BLOCK((blk=argumentList->object));
  CTR_ENSURE_NON_NULL(argumentList->next);
  me = argumentList->next->object;
  CTR_ENSURE_NON_NULL(argumentList->next->next);
  CTR_ENSURE_TYPE_ARRAY((argl=argumentList->next->next->object));

  ctr_object* arr     = argl;
  ctr_size length = (int) ctr_array_count( arr,  NULL )->value.nvalue;
  int i = 0;
  ctr_argument* args = ctr_heap_allocate( sizeof( ctr_argument ) );
  ctr_argument* cur  = args;
  for ( i = 0; i < length; i ++ ) {
      ctr_argument* index = ctr_heap_allocate( sizeof( ctr_argument ) );
      if ( i > 0 ) {
          cur->next = ctr_heap_allocate( sizeof( ctr_argument ) );
          cur = cur->next;
      }
      index->object = ctr_build_number_from_float( (double) i );
      cur->object = ctr_array_get( arr, index );
      ctr_heap_free( index );
  }
  ctr_object* answer = ctr_block_run(blk, args, me);
  cur = args;
  if ( length == 0 ) {
      ctr_heap_free(args);
  } else {
      for ( i = 0; i < length; i ++ ) {
          ctr_argument* a = cur;
          if ( i < length - 1 ) cur = cur->next;
          ctr_heap_free( a );
      }
  }
  return answer;
}

/**
 * [Reflect] runHere: [Block] forObject: [o:Object] arguments: [Array]
 *
 * runs a block with its 'me'/'my' set to object, without switching contexts
 **/
ctr_object* ctr_reflect_run_for_object_ctx(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object *blk, *me, *argl;
  CTR_ENSURE_TYPE_BLOCK((blk=argumentList->object));
  CTR_ENSURE_NON_NULL(argumentList->next);
  me = argumentList->next->object;
  CTR_ENSURE_NON_NULL(argumentList->next->next);
  CTR_ENSURE_TYPE_ARRAY((argl=argumentList->next->next->object));

  ctr_object* arr     = argl;
  ctr_size length = (int) ctr_array_count( arr,  NULL )->value.nvalue;
  int i = 0;
  ctr_argument* args = ctr_heap_allocate( sizeof( ctr_argument ) );
  ctr_argument* cur  = args;
  for ( i = 0; i < length; i ++ ) {
      ctr_argument* index = ctr_heap_allocate( sizeof( ctr_argument ) );
      if ( i > 0 ) {
          cur->next = ctr_heap_allocate( sizeof( ctr_argument ) );
          cur = cur->next;
      }
      index->object = ctr_build_number_from_float( (double) i );
      cur->object = ctr_array_get( arr, index );
      ctr_heap_free( index );
  }
  ctr_object* answer = ctr_block_run_here(blk, args, me);
  cur = args;
  if ( length == 0 ) {
      ctr_heap_free(args);
  } else {
      for ( i = 0; i < length; i ++ ) {
          ctr_argument* a = cur;
          if ( i < length - 1 ) cur = cur->next;
          ctr_heap_free( a );
      }
  }
  return answer;
}

/**
 * [Reflect] closure: [String] of: [Object]
 *
 * captures a method from an object as a closure
 *
 * (Reflect closure: 'closure:of:' of: Reflect) applyTo: 'toString' and: 10. #'10'
 **/
ctr_object* ctr_reflect_closure_of(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* methodName = argumentList->object;
  ctr_object* object = argumentList->next->object;
  ctr_object* methodObject = ctr_string_eval(ctr_build_string_from_cstring("{:*args ^my self message: my method arguments: args.}"), NULL);
  ctr_internal_object_set_property(methodObject, ctr_build_string_from_cstring("self"), object, CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(methodObject, ctr_build_string_from_cstring("method"), methodName, CTR_CATEGORY_PRIVATE_PROPERTY);
  return methodObject;
}

/**
 * [Reflect] getProperty: [p:String] ofObject: [o:Object]
 *
 * returns the property p of object o.
 * this will produce an error should said property not exist.
 *
 * Reflect getProperty: 'end' ofObject: Eval
 **/
ctr_object* ctr_reflect_get_property(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* name = ctr_internal_cast2string(argumentList->object);
  ctr_object* ns = argumentList->next->object;
  ctr_object* ret = ctr_internal_object_find_property(ns, name, 0);
  if (ret) return ret;
  ctr_object* ns_type = ns->lexical_name ? ns->lexical_name :  ctr_internal_cast2string(ctr_send_message(ns, "type", 4, NULL));
  int len = snprintf(NULL, 0, "Attempt to read nonexistent property '%.*s' of Object %p (namely '%.*s' in this context)", (int)(name->value.svalue->vlen), name->value.svalue->value, ns, (int)(ns_type->value.svalue->vlen), ns_type->value.svalue->value);
  char* buf = malloc(len);
  sprintf(buf, "Attempt to read nonexistent property '%.*s' of Object %p (namely '%.*s' in this context)", (int)(name->value.svalue->vlen), name->value.svalue->value, ns, (int)(ns_type->value.svalue->vlen), ns_type->value.svalue->value);
  CtrStdFlow = ctr_build_string(buf, len);
  free(buf);
  return CtrStdNil;
}

// #ifdef WITH_INSTRUMENTORS

/**
 * [Reflect] disableInstrumentation
 * Sends a message to an object with some arguments bypassing the instrumentor
 */
ctr_object* ctr_reflect_rawmsg(ctr_object* myself, ctr_argument* argumentList) {
  ctr_past_instrumentor_func = ctr_instrumentor_func;
  ctr_instrumentor_func = NULL;
  return myself;
}

/**
 * [Reflect] enableInstrumentation
 * Sends a message to an object with some arguments bypassing the instrumentor
 */
ctr_object* ctr_reflect_instrmsg(ctr_object* myself, ctr_argument* argumentList) {
  if(!ctr_instrumentor_func)
    ctr_instrumentor_func = ctr_past_instrumentor_func;
  return myself;
}

/**
 * [Reflect] registerInstrumentor: [Block<object, message, arguments>:<object, message, arguments>]
 *
 * register to an event that fires every time a message is sent.
 * This instrumentor will have to handle all message sending operations
 * using `[Reflect] disableInstrumentation` and '[Reflect] enableInstrumentation'.
 * return value is used as the result of the message, unless it is the instrumentor function,
 * In which case, the message is handed down to the object raw
 */
ctr_object* ctr_reflect_register_instrumentor(ctr_object* myself, ctr_argument* argumentList) {
  ctr_instrumentor_func = argumentList->object;
  return myself;
}

/**
 * [Reflect] currentInstrumentor
 *
 * gets the current instrumentor instance, or Nil if none exists
 */
ctr_object* ctr_reflect_get_instrumentor(ctr_object* myself, ctr_argument* argumentList) {
  if(!ctr_instrumentor_func) return ctr_build_nil();
  return ctr_instrumentor_func;
}

/**
 * @Experimental
 *
 * [Reflect] runAtGlobal: [Block] arguments: [[Object]]
 *
 * Runs a block at global context passing it the arguments
 */
ctr_object* ctr_reflect_run_glob(ctr_object* myself, ctr_argument* argumentList) {
  int old_index = ctr_context_id;
  ctr_context_id = 0;
  ctr_object* ret = ctr_block_run(argumentList->object, argumentList->next, argumentList->object);
  ctr_context_id = old_index;
  return ret;
}
///Trash v
ctr_object* ctr_reflect_cons_value(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("value"), 0);
}
ctr_object* ctr_reflect_cons_str(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_internal_cast2string(ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("value"), 0));
}
