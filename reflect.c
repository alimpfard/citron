#include <stdio.h>
#include "citron.h"


ctr_object* ctr_reflect_new(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = myself;
    return instance;
}

ctr_object* ctr_reflect_add_glob (ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = CtrStdObject;
    instance->info.sticky = 0;
    ctr_internal_object_add_property(CtrStdWorld, ctr_internal_cast2string(argumentList->object), instance, 0);
    return CtrStdNil;
}

ctr_object* ctr_reflect_add_local (ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = CtrStdObject;
    ctr_internal_object_add_property(ctr_contexts[ctr_context_id], ctr_internal_cast2string(argumentList->object), instance, CTR_CATEGORY_PUBLIC_PROPERTY);
    return CtrStdNil;
}
ctr_object* ctr_reflect_add_my (ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    instance->link = CtrStdObject;
    ctr_internal_object_add_property(ctr_contexts[ctr_context_id], ctr_internal_cast2string(argumentList->object), instance, CTR_CATEGORY_PRIVATE_PROPERTY);
    return CtrStdNil;
}
ctr_object* ctr_reflect_set_to(ctr_object* myself, ctr_argument* argumentList) {
    ctr_internal_object_set_property(CtrStdWorld, ctr_internal_cast2string(argumentList->object), argumentList->next->object, 0);
    return CtrStdNil;
}
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
ctr_object* ctr_reflect_dump_context_spec(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* of = argumentList->object;
    ctr_object* meths = ctr_array_new(CtrStdArray, NULL);
    int m = of->methods->size - 1;
    struct ctr_mapitem* head;
    head = of->methods->head;
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
    return meths;
}

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
ctr_object* ctr_reflect_find_obj(ctr_object* myself, ctr_argument* argumentList) {
    return ctr_find(ctr_internal_cast2string(argumentList->object));
}

ctr_object* ctr_reflect_extst_obj(ctr_object* myself, ctr_argument* argumentList) {
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
    args->object = ctr_build_string(param->value, param->vlen);
    ctr_send_message(arglist, "push:", 5, args);
    parameterList = parameterList->next;
    if(parameterList == NULL) break;
  }
  ctr_heap_free(args);
  return arglist;
}

ctr_object* ctr_reflect_cb_add_param(ctr_object* myself, ctr_argument* argumentList) {
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
  memcpy(parameterList->node->value, name, strlen(name));
  ctr_heap_free(name);
  parameterList->node->vlen = strlen(name);
  return myself;
}
ctr_object* ctr_reflect_fn_copy(ctr_object* myself, ctr_argument* argumentList) {
  if(argumentList->object->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
    CtrStdFlow = ctr_build_string_from_cstring("parameter was of an invalid type.");
    return CtrStdNil;
  }
  ctr_object* newblk = ctr_build_block(argumentList->object->value.block);
  ctr_object* name = ctr_build_string_from_cstring("inheritance");
  ctr_object* prop = ctr_internal_object_find_property(argumentList->object, name, CTR_CATEGORY_PRIVATE_PROPERTY);
  if(prop != NULL) ctr_internal_object_add_property(newblk, name, prop, CTR_CATEGORY_PRIVATE_PROPERTY);
  return newblk;
}

// ctr_object* ctr_reflect_obj_hp(ctr_object* myself, ctr_argument* argumentList) {
//     ctr_object* object = argumentList->object;
//     return ctr_internal_cast2string(object->);
// }

// void begin() {
//     ctr_object* reflect = ctr_reflect_new(CtrStdObject, NULL);
//
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("addGlobalVariable:"), &ctr_reflect_add_glob);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("addLocalVariable:"), &ctr_reflect_add_local);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("addPrivateVariable:"), &ctr_reflect_add_my);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("set:to:"), &ctr_reflect_set_to);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("getContext"), &ctr_reflect_dump_context);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("getMethodsOf:"), &ctr_reflect_dump_context_spec);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("getPropertiesOf:"), &ctr_reflect_dump_context_spec_prop);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("getObject:"), &ctr_reflect_find_obj);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("objectExists:"), &ctr_reflect_extst_obj);
//     ctr_internal_create_func(reflect, ctr_build_string_from_cstring("argumentListOf:"), &ctr_reflect_cb_ac);
//
//     ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("Reflect"), reflect, 0);
// }
