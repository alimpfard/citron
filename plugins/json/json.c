#include "../../citron.h"
#include "jansson.h"

ctr_object* ctr_string_dquotes_escape(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* answer;
    char* str;
    ctr_size len;
    ctr_size i;
    ctr_size j;
    len = myself->value.svalue->vlen;
    for( i = 0; i < myself->value.svalue->vlen; i++ ) {
        if ( *(myself->value.svalue->value + i) == '"' ) {
            len++;
        }
    }
    str = ctr_heap_allocate( len + 1 );
    j = 0;
    for( i = 0; i < myself->value.svalue->vlen; i++ ) {
        if ( *(myself->value.svalue->value + i) == '"' ) {
            str[j+i] = '\\';
            j++;
        }
        str[j+i] = *(myself->value.svalue->value + i);
    }
    answer = ctr_build_string_from_cstring( str );
    ctr_heap_free( str );
    return answer;
}

ctr_object* ctr_json_create_object(json_t* root, ctr_object* gt) {
    switch(json_typeof(root)) {
        case JSON_OBJECT: {
            ctr_object* sub = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
            sub->link = gt;
            // size_t size;
            const char *key;
            json_t *value;
            // size = json_object_size(root);
            json_object_foreach(root, key, value) {
              char* k = (char*)key;
              ctr_internal_object_add_property(sub, ctr_build_string_from_cstring(k), ctr_json_create_object((json_t*)value, gt), CTR_CATEGORY_PRIVATE_PROPERTY);
            }
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
          ctr_object* str = ctr_build_string_from_cstring((char*)json_string_value(root));
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
json_t *load_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        char* err = "";
        sprintf(err, "json error on line %d: %s\n", error.line, error.text);
        CtrStdFlow = ctr_build_string_from_cstring(err);
        return (json_t *)0;
    }
}
ctr_object* ctr_serialize_map(ctr_object* object);
ctr_object* ctr_json_serialize_(ctr_object* object);

ctr_object* ctr_json_parse (ctr_object* myself, ctr_argument* argumentList) {
  char* jso = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
  json_t* root = load_json(jso);
  ctr_heap_free(jso);
  if(root) {
    ctr_object* obj = ctr_json_create_object(root, argumentList->next->object);
    if(!obj->link) obj->link = argumentList->next->object;
    return obj;
  } else return CtrStdNil;
}
ctr_object* ctr_json_serialize_(ctr_object* object) {
  ctr_object* obj;
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
      for(i=object->value.avalue->tail; i<object->value.avalue->head; i++) {
          arrayElement = *( object->value.avalue->elements + i );
          if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL || arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER
                  || arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL ) {
              newArgumentList->object = ctr_json_serialize_(arrayElement);
              string = ctr_string_append( string, newArgumentList );
          } else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
              newArgumentList->object = ctr_build_string_from_cstring("\"");
              string = ctr_string_append( string, newArgumentList );
              newArgumentList->object = ctr_string_quotes_escape( arrayElement, newArgumentList );
              string = ctr_string_append( string, newArgumentList );
              newArgumentList->object = ctr_build_string_from_cstring("\"");
              string = ctr_string_append( string, newArgumentList );
          } else {
              newArgumentList->object = ctr_build_string_from_cstring("(");
              ctr_string_append( string, newArgumentList );
              newArgumentList->object = ctr_json_serialize_(arrayElement);
              string = ctr_string_append( string, newArgumentList );
              newArgumentList->object = ctr_build_string_from_cstring(")");
              ctr_string_append( string, newArgumentList );
          }
          if (  (i + 1 )<object->value.avalue->head ) {
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
      CtrStdFlow = ctr_build_string_from_cstring("Cannot serialize this object."); //TODO:get a decent message.
      return CtrStdNil;
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
  while( mapItem ) {
      newArgumentList->object = ctr_build_string_from_cstring("\"");
      ctr_string_append( string, newArgumentList );
      newArgumentList->object = ctr_internal_cast2string(mapItem->key);
      ctr_string_append( string, newArgumentList );
      newArgumentList->object = ctr_build_string_from_cstring("\": ");
      ctr_string_append( string, newArgumentList );
      newArgumentList->object = ctr_json_serialize_(mapItem->value);
      ctr_string_append( string, newArgumentList );
      mapItem = mapItem->next;
      if ( mapItem ) {
          newArgumentList->object = ctr_build_string_from_cstring( ", " );
          ctr_string_append( string, newArgumentList );
      }
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
  jans->link = CtrStdObject;
  ctr_internal_create_func(jans, ctr_build_string_from_cstring("parse:genericType:"), &ctr_json_parse);
  ctr_internal_create_func(jans, ctr_build_string_from_cstring("serialize:"), &ctr_json_serialize);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("JSON"), jans, 0);
}
