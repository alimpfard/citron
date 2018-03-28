#include "../../citron.h"
#include "jansson.h"

ctr_object* ctr_json_create_object(json_t* root, ctr_object* gt) {
    switch(json_typeof(root)) {
        case JSON_OBJECT: {
            ctr_object* sub = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
            ctr_set_link_all(sub, gt);
            // size_t size;
            const char *key;
            json_t *value;
            ctr_argument* argl = ctr_heap_allocate(sizeof(*argl));
            argl->next = ctr_heap_allocate(sizeof(*argl));
            // size = json_object_size(root);
            json_object_foreach(root, key, value) {
              char* k = (char*)key;
              ctr_object* ko = ctr_build_string_from_cstring(k);
              ctr_object* vo = ctr_json_create_object(value, gt);
              argl->object = vo;
              argl->next->object = ko;
              sub = ctr_map_put(sub, argl);
            }
            ctr_heap_free(argl->next);
            ctr_heap_free(argl);
            return sub;
          }
        case JSON_ARRAY: {
          ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
          ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
          size_t i;
          size_t size = json_array_size(root);
          for (i = 0; i < size; i++) {
            arg->object = ctr_json_create_object(json_array_get(root, i), gt);
            ctr_array_push(arr, arg);
          }
          ctr_heap_free(arg);
          return arr;
        }
        case JSON_STRING: {
          ctr_object* str = ctr_build_string((char*)json_string_value(root), json_string_length(root));
          return str;
        }
        case JSON_INTEGER: {
          return ctr_build_number_from_float(json_integer_value(root));
        }
        case JSON_REAL: {
          return ctr_build_number_from_float(json_real_value(root));
        }
        case JSON_FALSE: {
          return ctr_build_bool(0);
        }
        case JSON_TRUE: {
          return ctr_build_bool(1);
        }
        case JSON_NULL: {
          return ctr_build_nil();
        }
        default: {
          CtrStdFlow = ctr_build_string_from_cstring("Unrecognized JSON type");
          return CtrStdNil;
        }
    }
}
json_t *load_json(const char *text, const char* fp) {
    json_t *root;
    json_error_t error;

    root = text ? json_loads(text, 0, &error) : json_load_file(fp, 0, &error);

    if (root) {
        return root;
    } else {
        char err[2048];
        sprintf(err, "json error on line %d: %s", error.line, error.text);
        CtrStdFlow = ctr_build_string_from_cstring(err);
        return (json_t *)0;
    }
}
ctr_object* ctr_serialize_map(ctr_object* object);
ctr_object* ctr_json_serialize_(ctr_object* object);

ctr_object* ctr_json_parse (ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* link = ctr_reflect_get_primitive_link(argumentList->object);
  json_t* root;
  if(link == CtrStdString) {
    char* jso = ctr_heap_allocate_cstring(argumentList->object);
    root = load_json(jso, 0);
    ctr_heap_free(jso);
  } else if (link == CtrStdFile) {
    char* jso = ctr_heap_allocate_cstring(ctr_file_rpath(argumentList->object, 0));
    root = load_json(0, jso);
    ctr_heap_free(jso);
  } else {
    char* jso = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
    root = load_json(jso, 0);
    ctr_heap_free(jso);
  }
  ctr_object* p = argumentList->next->object ? argumentList->next->object : CtrStdMap;
  if(root) {
    ctr_object* obj = ctr_json_create_object(root, p);
    if(!obj->interfaces->link) ctr_set_link_all(obj, p);
    return obj;
  } else return CtrStdNil;
}
ctr_object* ctr_json_serialize_(ctr_object* object) {
  ctr_object* obj;
  if(!object) return ctr_build_string_from_cstring("null");
  switch (object->info.type) {
    case CTR_OBJECT_TYPE_OTNIL: {
      obj = ctr_build_string_from_cstring("null");
      break;
    }
    case CTR_OBJECT_TYPE_OTARRAY: {
      int i;
      ctr_object* arrayElement;
      ctr_argument* newArgumentList;
      ctr_object* string = ctr_build_empty_string();
      newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
      newArgumentList->object = ctr_build_string_from_cstring( "[" );
      string = ctr_string_append( string, newArgumentList );
      int size = ctr_array_count(object, NULL)->value.nvalue;
      for(i=0; i<size; i++) {
          newArgumentList->object = ctr_build_number_from_float(i);
          arrayElement = ctr_array_get(object, newArgumentList);
          // if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL || arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER
          //         || arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL ) {
          //     newArgumentList->object = ctr_json_serialize_(arrayElement);
          //     string = ctr_string_append( string, newArgumentList );
          // } else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
          //     newArgumentList->object = ctr_build_string_from_cstring("\"");
          //     string = ctr_string_append( string, newArgumentList );
          //     newArgumentList->object = ctr_string_dquotes_escape( arrayElement, newArgumentList );
          //     string = ctr_string_append( string, newArgumentList );
          //     newArgumentList->object = ctr_build_string_from_cstring("\"");
          //     string = ctr_string_append( string, newArgumentList );
          // } else {
          //     //newArgumentList->object = ctr_build_string_from_cstring("(");
          //     //ctr_string_append( string, newArgumentList );
          //     newArgumentList->object = ctr_json_serialize_(arrayElement);
          //     string = ctr_string_append( string, newArgumentList );
          //     //newArgumentList->object = ctr_build_string_from_cstring(")");
          //     //ctr_string_append( string, newArgumentList );
          // }
          newArgumentList->object = ctr_json_serialize_(arrayElement);
          string = ctr_string_append( string, newArgumentList );
          if (  (i + 1)<object->value.avalue->head ) {
              newArgumentList->object = ctr_build_string_from_cstring(", ");
              string = ctr_string_append( string, newArgumentList );
          }
      }
      newArgumentList->object = ctr_build_string_from_cstring("]");
      string = ctr_string_append( string, newArgumentList );

      ctr_heap_free( newArgumentList );
      return string;
    }
    case CTR_OBJECT_TYPE_OTOBJECT: {
      if(ctr_internal_has_responder(object, ctr_build_string_from_cstring("toJSON")))
        obj = ctr_internal_cast2string(ctr_send_message(object, "toJSON", 6, NULL));
      else
        obj = ctr_serialize_map(object);
      break;
    }
    case CTR_OBJECT_TYPE_OTNUMBER: {
      obj = ctr_internal_cast2string(object);
      break;
    }
    case CTR_OBJECT_TYPE_OTSTRING: {
      ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
      args->object = ctr_string_dquotes_escape(object, NULL);
      obj = ctr_build_string_from_cstring("\"");
      ctr_string_append(obj,args);
      args->object = ctr_build_string_from_cstring("\"");
      ctr_string_append(obj, args);
      ctr_heap_free(args); break;
    }
    case CTR_OBJECT_TYPE_OTBOOL: {
      return ctr_build_string_from_cstring((object->value.bvalue)?"true":"false");
      break;
    }
    default: {
      if(ctr_internal_has_responder(object, ctr_build_string_from_cstring("toJSON")))
        obj = ctr_internal_cast2string(ctr_send_message(object, "toJSON", 6, NULL));
      else {
        CtrStdFlow = ctr_build_string_from_cstring("Cannot serialize this object, implement the message toJSON"); //TODO:get a decent message.
        return CtrStdNil;
      }
      break;
    }
  }
  return obj;
}
ctr_object* ctr_serialize_map(ctr_object* object) {
  ctr_object*  string;
  ctr_mapitem* mapItem;
  ctr_argument* newArgumentList;
  string  = ctr_build_string_from_cstring( "{" );
  mapItem = object->properties->head;
  newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
  int first = 1;
  while( mapItem ) {
      int skip = (mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING) && (strncmp(mapItem->key->value.svalue->value, "me", mapItem->key->value.svalue->vlen) == 0 || strncmp(mapItem->key->value.svalue->value, "thisBlock", mapItem->key->value.svalue->vlen) == 0);
      if(!skip) {
        if ( mapItem && !first ) {
            newArgumentList->object = ctr_build_string_from_cstring( ", " );
            ctr_string_append( string, newArgumentList );
        }
        if(first) first=0;
        newArgumentList->object = ctr_json_serialize_(ctr_internal_cast2string(mapItem->key));
        ctr_string_append( string, newArgumentList );
        newArgumentList->object = ctr_build_string_from_cstring(": ");
        ctr_string_append( string, newArgumentList );
        newArgumentList->object = ctr_json_serialize_(mapItem->value);
        ctr_string_append( string, newArgumentList );
      }
      mapItem = mapItem->next;
  }
  newArgumentList->object = ctr_build_string_from_cstring( "}" );
  ctr_string_append( string, newArgumentList );
  ctr_heap_free( newArgumentList );
  return string;
}

ctr_object* ctr_json_serialize(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_json_serialize_(argumentList->object);
}

void begin() {
  ctr_object* jans = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(jans, CtrStdObject);
  ctr_internal_create_func(jans, ctr_build_string_from_cstring("parse:genericType:"), &ctr_json_parse);
  ctr_internal_create_func(jans, ctr_build_string_from_cstring("parse:"), &ctr_json_parse);
  ctr_internal_create_func(jans, ctr_build_string_from_cstring("serialize:"), &ctr_json_serialize);
  ctr_internal_create_func(jans, ctr_build_string_from_cstring("dump:"), &ctr_json_serialize);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("JSON"), jans, 0);
}
