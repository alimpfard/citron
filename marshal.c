#include <stdio.h>
#include "citron.h"

// FORMAT <type> [<size/length>] [m <length> <metadata_methods>] [p <length> <metadata_properties>] [<value(s)>]

#define CTR_MARSH_OTNIL           'N'
#define CTR_MARSH_OTBOOL          '?'
#define CTR_MARSH_OTNUMBER        'D'
#define CTR_MARSH_OTSTRING        'S'
#define CTR_MARSH_OTBLOCKL        'b'
#define CTR_MARSH_OTBLOCKNL       'B'
#define CTR_MARSH_OTOBJECT        'O'
#define CTR_MARSH_OTNATFUNC       'P'
#define CTR_MARSH_OTARRAY         'A'
#define CTR_MARSH_OTMISC          'M'
#define CTR_MARSH_OTEX            'E'
#define CTR_MARSH_META_METH       'm'
#define CTR_MARSH_META_PROP       'p'
#define CTR_MARSH_NMAP            ':'

#define MARSH_BUFFER_ENLARGEMENT_FACTOR 2

#define CTR_MARSH_AST_EXPRASSIGNMENT '='
#define CTR_MARSH_AST_EXPRMESSAGE '>'
#define CTR_MARSH_AST_UNAMESSAGE '"'
#define CTR_MARSH_AST_BINMESSAGE '!'
#define CTR_MARSH_AST_KWMESSAGE 'K'
#define CTR_MARSH_AST_LTRSTRING 'S'
#define CTR_MARSH_AST_REFERENCE 'R'
#define CTR_MARSH_AST_LTRNUM 'D'
#define CTR_MARSH_AST_CODEBLOCKL 'b'
#define CTR_MARSH_AST_CODEBLOCKNL 'B'
#define CTR_MARSH_AST_RETURNFROMBLOCK '^'
#define CTR_MARSH_AST_IMMUTABLE '['
#define CTR_MARSH_AST_SYMBOL '\\'
#define CTR_MARSH_AST_PARAMLIST ':'
#define CTR_MARSH_AST_PARAM ';'
#define CTR_MARSH_AST_INSTRLIST '-'
#define CTR_MARSH_AST_ENDOFPROGRAM 'E'
#define CTR_MARSH_AST_NESTED '('
#define CTR_MARSH_AST_LTRBOOLTRUE 'T'
#define CTR_MARSH_AST_LTRBOOLFALSE 'F'
#define CTR_MARSH_AST_LTRNIL 'N'
#define CTR_MARSH_AST_PROGRAM '+'

#define CTR_MARSH_OBJECT_LINK_Object         'o'
#define CTR_MARSH_OBJECT_LINK_Block          'b'
#define CTR_MARSH_OBJECT_LINK_String         's'
#define CTR_MARSH_OBJECT_LINK_Number         'n'
#define CTR_MARSH_OBJECT_LINK_Bool           'B'
#define CTR_MARSH_OBJECT_LINK_Console        'c'
#define CTR_MARSH_OBJECT_LINK_Nil            'N'
#define CTR_MARSH_OBJECT_LINK_GC             'g'
#define CTR_MARSH_OBJECT_LINK_Map            'M'
#define CTR_MARSH_OBJECT_LINK_Array          'a'
#define CTR_MARSH_OBJECT_LINK_Iter           'i'
#define CTR_MARSH_OBJECT_LINK_File           'f'
#define CTR_MARSH_OBJECT_LINK_System         'S'
#define CTR_MARSH_OBJECT_LINK_Dice           'd'
#define CTR_MARSH_OBJECT_LINK_Command        'C'
#define CTR_MARSH_OBJECT_LINK_Slurp          'l'
#define CTR_MARSH_OBJECT_LINK_Shell          'h'
#define CTR_MARSH_OBJECT_LINK_Clock          'k'
#define CTR_MARSH_OBJECT_LINK_Reflect        'r'
#define CTR_MARSH_OBJECT_LINK_Reflect_cons   '_'
#define CTR_MARSH_OBJECT_LINK_Fiber          'F'
#define CTR_MARSH_OBJECT_LINK_Thread         'T'
#define CTR_MARSH_OBJECT_LINK_Symbol         '\\'
#define CTR_MARSH_OBJECT_LINK_DESCRIBE       'x'

ctr_size ctr_marshal_object(ctr_object*, char**, ctr_size*, ctr_size*);
void ctr_marshal_store_link(ctr_object*, char**, ctr_size*, ctr_size*);
ctr_object* ctr_marshal_resolve_linq(char*, ctr_size*, ctr_size*);
void ctr_marshal_keyvalues(ctr_mapitem*, ctr_size, char**, ctr_size*, ctr_size*);
ctr_object* ctr_unmarshal_object(char* stream, ctr_size* avail, ctr_size* consumed);

ctr_mapitem* get_last_item(ctr_map* map) {
  if(!map) return NULL;
  ctr_mapitem* item = map->head;
  for(size_t len=map->size; len>1; len--) {
    item = item->next;
  }
  return item;
}

ctr_tnode* ctr_unmarshal_ast(char* stream, size_t avail, size_t* consumed) {
  ctr_tnode* node = ctr_heap_allocate(sizeof(*node));
  char type = *(stream+((*consumed)++));

  switch (type) {
    case CTR_MARSH_AST_EXPRASSIGNMENT: {
      ctr_tnode* assignee = ctr_unmarshal_ast(stream, avail, consumed);
      ctr_tnode* value = ctr_unmarshal_ast(stream, avail, consumed);
      node->type = CTR_AST_NODE_EXPRASSIGNMENT;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = assignee;
      node->nodes->next->node = value;
    break;
    }
    case CTR_MARSH_AST_EXPRMESSAGE: {
      node->type = CTR_AST_NODE_EXPRMESSAGE;
      ctr_tnode* receiver = ctr_unmarshal_ast(stream, avail, consumed);
      size_t msg_count;
      memcpy(&msg_count, stream+*consumed, sizeof(size_t));
      *consumed += sizeof(size_t);
      ctr_tlistitem* msgs = ctr_heap_allocate(sizeof(ctr_tlistitem));
      ctr_tlistitem* msgs_ = msgs;
      for(;msg_count>0;msg_count--) {
        msgs->node = ctr_unmarshal_ast(stream, avail, consumed);
        if(msg_count>1) {
          msgs->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
          msgs=msgs->next;
        }
      }
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = receiver;
      node->nodes->next = msgs_;
    break;
    }
    case CTR_MARSH_AST_UNAMESSAGE: {
      node->type = CTR_AST_NODE_UNAMESSAGE;
      size_t len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed += sizeof(size_t);
      node->value = ctr_heap_allocate(sizeof(char)*len);
      node->vlen = len;
      memcpy(node->value, stream+*consumed, len);
      *consumed+=len;
    break;
    }
    case CTR_MARSH_AST_BINMESSAGE: {
      node->type = CTR_AST_NODE_BINMESSAGE;
      size_t len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed += sizeof(size_t);
      node->value = ctr_heap_allocate(sizeof(char)*len);
      node->vlen = len;
      memcpy(node->value, stream+*consumed, len);
      *consumed+=len;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = ctr_unmarshal_ast(stream, avail, consumed);
    break;
    }
    case CTR_MARSH_AST_KWMESSAGE: {//KWMESSAGE <msg_len> <message> <argument_count> (<argument>)+
      node->type = CTR_AST_NODE_KWMESSAGE;
      size_t len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed += sizeof(size_t);
      node->value = ctr_heap_allocate(sizeof(char)*len);
      memcpy(node->value, stream+*consumed, len);
      *consumed += len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed += sizeof(size_t);
      ctr_tlistitem* li = node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      for(;len>0;len--) {
        li->node = ctr_unmarshal_ast(stream, avail, consumed);
        if(len>1) {
          li->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
          li = li->next;
        }
      }
    break;
    }
    case CTR_MARSH_AST_LTRSTRING: {
      node->type = CTR_AST_NODE_LTRSTRING;
      size_t len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed+=sizeof(size_t);
      node->value = ctr_heap_allocate(sizeof(char)*len);
      memcpy(node->value, stream+*consumed, len);
      node->vlen = len;
      *consumed += len;
    break;
    }
    case CTR_MARSH_AST_REFERENCE: {
      node->type = CTR_AST_NODE_REFERENCE;
      node->modifier = *(stream+((*consumed)++));
      size_t len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed+=sizeof(size_t);
      node->value = ctr_heap_allocate(sizeof(char)*len);
      memcpy(node->value, stream+*consumed, len);
      node->vlen = len;
      *consumed += len;
    break;
    }
    case CTR_MARSH_AST_LTRNUM: {
      node->type = CTR_AST_NODE_LTRNUM;
      size_t len;
      memcpy(&len, stream+*consumed, sizeof(size_t));
      *consumed+=sizeof(size_t);
      node->value = ctr_heap_allocate(sizeof(char)*len);
      memcpy(node->value, stream+*consumed, len);
      node->vlen = len;
      *consumed += len;
    break;
    }
    case CTR_MARSH_AST_PARAMLIST: {
      node->type = CTR_AST_NODE_PARAMLIST;
      ctr_size plen;
      memcpy(&plen, stream+*consumed, sizeof(ctr_size));
      *consumed+=sizeof(ctr_size);
      ctr_tlistitem* nl = ctr_heap_allocate(sizeof(ctr_tlistitem));
      ctr_tlistitem* onl = nl;
      for(;plen>0;plen--) {
        nl->node = ctr_unmarshal_ast(stream, avail, consumed);
        if(plen>1) {
          nl->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
          nl = nl->next;
        }
      }
      node->nodes = onl;
    break;
    }
    case CTR_MARSH_AST_CODEBLOCKL: {
      node->type = CTR_AST_NODE_CODEBLOCK;
      node->lexical = 1;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = ctr_unmarshal_ast(stream, avail, consumed);
      node->nodes->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->next->node = ctr_unmarshal_ast(stream, avail, consumed);
    break;
    }
    case CTR_MARSH_AST_CODEBLOCKNL: {
      node->type = CTR_AST_NODE_CODEBLOCK;
      node->lexical = 0;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = ctr_unmarshal_ast(stream, avail, consumed);
      node->nodes->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->next->node = ctr_unmarshal_ast(stream, avail, consumed);
    break;
    }
    case CTR_MARSH_AST_RETURNFROMBLOCK: {
      node->type = CTR_AST_NODE_RETURNFROMBLOCK;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = ctr_unmarshal_ast(stream, avail, consumed);
    break;
    }
    case CTR_MARSH_AST_IMMUTABLE: {
    break;
    }
    case CTR_MARSH_AST_SYMBOL: {
    break;
    }
    case CTR_MARSH_AST_PARAM: {
      ctr_size len;
      memcpy(&len, stream+*consumed, sizeof(ctr_size));
      *consumed += sizeof(len);
      node->type = 0;
      node->value = ctr_heap_allocate(sizeof(char)*len);
      memcpy(node->value, stream+*consumed, len);
      node->vlen = len;
      *consumed += len;
    break;
    }
    case CTR_MARSH_AST_INSTRLIST: {
      ctr_size plen;
      ctr_tlistitem* nl;
      node->type = CTR_AST_NODE_INSTRLIST;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      memcpy(&plen, stream+*consumed, sizeof(ctr_size));
      *consumed+=sizeof(ctr_size);
      nl = node->nodes;
      for(;plen>0;plen--) {
        nl->node = ctr_unmarshal_ast(stream, avail, consumed);
        if(plen>1) {
          nl->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
          nl = nl->next;
        }
      }
    break;
    }
    case CTR_MARSH_AST_ENDOFPROGRAM: {
      node->type = CTR_AST_NODE_ENDOFPROGRAM;
    break;
    }
    case CTR_MARSH_AST_NESTED: {
      node->type = CTR_AST_NODE_NESTED;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      node->nodes->node = ctr_unmarshal_ast(stream, avail, consumed);
    break;
    }
    case CTR_MARSH_AST_LTRBOOLTRUE:
    case CTR_MARSH_AST_LTRBOOLFALSE: {
      char v = stream[(*consumed)++];
      node->type = v == 1 ? CTR_AST_NODE_LTRBOOLTRUE : CTR_AST_NODE_LTRBOOLFALSE;
    break;
    }
    case CTR_MARSH_AST_LTRNIL: {
      node->type = CTR_AST_NODE_LTRNIL;
    break;
    }
    case CTR_MARSH_AST_PROGRAM: {
      ctr_size plen;
      ctr_tlistitem* nl;
      node->type = CTR_AST_NODE_PROGRAM;
      node->nodes = ctr_heap_allocate(sizeof(ctr_tlistitem));
      memcpy(&plen, stream+*consumed, sizeof(ctr_size));
      *consumed+=sizeof(ctr_size);
      nl = node->nodes;
      for(;plen>0;plen--) {
        nl->node = ctr_unmarshal_ast(stream, avail, consumed);
        if(plen>1) {
          nl->next = ctr_heap_allocate(sizeof(ctr_tlistitem));
          nl = nl->next;
        }
      }
    }
  }
  return node;
}

void ctr_marshal_ast(ctr_tnode* rnode, char** stream, size_t* stream_len, size_t* used) {
  // printf("HERE WE GO BOIS %p(%d)\n", rnode, rnode->type);
  if(*stream_len-*used < 2+sizeof(size_t)) {
    *stream_len *= MARSH_BUFFER_ENLARGEMENT_FACTOR;
    ctr_heap_reallocate(*stream, *stream_len);
  }
  switch(rnode->type) {
    case CTR_AST_NODE_EXPRASSIGNMENT: {//ASSIGN <expr> <expr>
      *(*stream+((*used)++)) = CTR_MARSH_AST_EXPRASSIGNMENT;
      ctr_marshal_ast(rnode->nodes->node, stream, stream_len, used);
      ctr_marshal_ast(rnode->nodes->next->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_EXPRMESSAGE: {//EXPRMESSAGE <receiver> <msg_node_count> (<message>)*
      size_t message_count = 0;
      *(*stream+((*used)++)) = CTR_MARSH_AST_EXPRMESSAGE;
      ctr_tlistitem* nl = rnode->nodes;
      while(nl&&nl->node) {
        message_count++;
        nl = nl->next;
      }
      nl = rnode->nodes;
      ctr_marshal_ast(nl->node, stream, stream_len, used);
      nl = nl->next;
      message_count--;
      memcpy(*stream+(*used), &message_count, sizeof(size_t));
      *used += sizeof(size_t);
      for(;message_count>0;message_count--,nl=nl->next)
        ctr_marshal_ast(nl->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_UNAMESSAGE: {//UNAMESSAGE <message_len> <message_name>
      memset(*stream+((*used)++), CTR_MARSH_AST_UNAMESSAGE, 1);
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      if(*stream_len-*used <= rnode->vlen) {
        *stream_len *= MARSH_BUFFER_ENLARGEMENT_FACTOR;
        ctr_heap_reallocate(*stream, *stream_len);
      }
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
    break;
    }
    case CTR_AST_NODE_BINMESSAGE: {//BINMESSAGE <msg_len(in case of ::)> <msg> <argument> (count = 1 implicit)
      memset(*stream+((*used)++), CTR_MARSH_AST_BINMESSAGE, 1);
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      if(*stream_len-*used <= rnode->vlen) {
        *stream_len *= MARSH_BUFFER_ENLARGEMENT_FACTOR;
        ctr_heap_reallocate(*stream, *stream_len);
      }
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
      ctr_marshal_ast(rnode->nodes->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_KWMESSAGE: {//KWMESSAGE <msg_len> <message> <argument_count> (<argument>)+
      *(*stream+((*used)++)) = CTR_MARSH_AST_KWMESSAGE;
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      if(*stream_len-*used <= rnode->vlen) {
        *stream_len *= MARSH_BUFFER_ENLARGEMENT_FACTOR;
        ctr_heap_reallocate(*stream, *stream_len);
      }
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
      ctr_size argcount = 0;
      ctr_tlistitem* msgns = rnode->nodes;
      while(msgns&&msgns->node) {
        argcount++;
        msgns = msgns->next;
      }
      memcpy(*stream+*used, &argcount, sizeof(ctr_size));
      *used += sizeof(size_t);
      msgns = rnode->nodes;
      for(;argcount>0;argcount--,msgns=msgns->next) {
        ctr_marshal_ast(msgns->node, stream, stream_len, used);
      }
    break;
    }
    case CTR_AST_NODE_LTRSTRING: {//STRING <length> <contents>
      memset(*stream+((*used)++), CTR_MARSH_AST_LTRSTRING, 1);
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
    break;
    }
    case CTR_AST_NODE_REFERENCE: {//REFERENCE <modifier> <length> <name>
      *(*stream+((*used)++)) = CTR_MARSH_AST_REFERENCE;
      *(*stream+((*used)++)) = rnode->modifier;
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
    break;
    }
    case CTR_AST_NODE_LTRNUM: {//LTRNUM <length> <value>
      memset(*stream+((*used)++), CTR_MARSH_AST_LTRNUM, 1);
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
    break;
    }
    case CTR_AST_NODE_RETURNFROMBLOCK: {// RETURN <ast>
      memset(*stream+((*used)++),  CTR_MARSH_AST_RETURNFROMBLOCK, 1);
      ctr_marshal_ast(rnode->nodes->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_IMMUTABLE: {

    break;
    }
    case CTR_AST_NODE_SYMBOL: {

    break;
    }
    case CTR_AST_NODE_PARAMLIST: {//PARAMLIST <count> (<argument>)*
      memset(*stream+((*used)++), CTR_MARSH_AST_PARAMLIST, 1);
      size_t count = 0;
      ctr_tlistitem* nl = rnode->nodes;
      while(nl&&nl->node) {
        count++;
        nl = nl->next;
      }
      memcpy(*stream+(*used), &count, sizeof(size_t));
      *used += sizeof(size_t);
      nl = rnode->nodes;
      for(;count>0;count--,nl=nl->next)
        ctr_marshal_ast(nl->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_INSTRLIST: {//INSTRLIST <count> (<instruction>)*
      memset(*stream+((*used)++), CTR_MARSH_AST_INSTRLIST, 1);
      size_t count = 0;
      ctr_tlistitem* nl = rnode->nodes;
      while(nl&&nl->node) {
        count++;
        nl = nl->next;
      }
      memcpy(*stream+(*used), &count, sizeof(size_t));
      *used += sizeof(size_t);
      nl = rnode->nodes;
      for(;count>0;count--,nl=nl->next)
        ctr_marshal_ast(nl->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_ENDOFPROGRAM: {
      memset(*stream+((*used)++), CTR_MARSH_AST_ENDOFPROGRAM, 1);
    break;
    }
    case CTR_AST_NODE_NESTED: {
      memset(*stream+((*used)++), CTR_MARSH_AST_NESTED, 1);
      ctr_marshal_ast(rnode->nodes->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_LTRBOOLTRUE: {
      memset(*stream+((*used)++), CTR_MARSH_AST_LTRBOOLTRUE, 1);
    break;
    }
    case CTR_AST_NODE_LTRBOOLFALSE: {
      memset(*stream+((*used)++), CTR_MARSH_AST_LTRBOOLFALSE, 1);
    break;
    }
    case CTR_AST_NODE_LTRNIL: {
      memset(*stream+((*used)++), CTR_MARSH_AST_LTRNIL, 1);
    break;
    }
    case CTR_AST_NODE_CODEBLOCK: {// CODEBLOCK (<argument>)* (<instruction>)*
      memset(*stream+((*used)++), rnode->lexical?CTR_MARSH_AST_CODEBLOCKL:CTR_MARSH_AST_CODEBLOCKNL, 1);
      // *Params
      ctr_tlistitem* np = rnode->nodes;
      ctr_marshal_ast(np->node, stream, stream_len, used);

      // *Instructions
      ctr_tlistitem* nl = rnode->nodes->next;
      ctr_marshal_ast(nl->node, stream, stream_len, used);
    break;
    }
    case CTR_AST_NODE_PROGRAM: {// PROGRAM (<instruction>)*
      memset(*stream+((*used)++), CTR_MARSH_AST_PROGRAM, 1);
      size_t instrc = 0;
      ctr_tlistitem* nl = rnode->nodes;
      while(nl&&nl->node) {
        instrc++;
        nl = nl->next;
      }
      memcpy(*stream+(*used), &instrc, sizeof(size_t));
      *used += sizeof(size_t);
      nl = rnode->nodes;
      for(;instrc>0;instrc--,nl=nl->next)
        ctr_marshal_ast(nl->node, stream, stream_len, used);
    break;
    }
    case 0: {//PARAMETER <length> <name>
      memset(*stream+((*used)++), CTR_MARSH_AST_PARAM, 1);
      memcpy(*stream+*used, &rnode->vlen, sizeof(size_t));
      *used += sizeof(size_t);
      memcpy(*stream+*used, rnode->value, rnode->vlen);
      *used += rnode->vlen;
    }
  }
}

void ctr_marshal_meta_object(ctr_object* obj, char** stream, ctr_size* stream_len, ctr_size* used) {//don't pass bullshit
  ctr_map *props = obj->properties, *meths = obj->methods;
  if(props->size > 0) {
    *(*stream+((*used)++)) = CTR_MARSH_META_PROP;
    ctr_size blen = 1024, bused=0;
    char* buffer = ctr_heap_allocate(blen);
    ctr_marshal_keyvalues(props->head, props->size, &buffer, &blen, &bused);
    memcpy((*stream+*used), &bused, sizeof(ctr_size));
    *used += sizeof(ctr_size);
    memcpy((*stream+*used), buffer, bused);
    *used += bused;
    ctr_heap_free(buffer);
  }
  if(meths->size > 0) {
    *(*stream+((*used)++)) = CTR_MARSH_META_METH;
    ctr_size blen = 1024, bused=0;
    char* buffer = ctr_heap_allocate(blen);
    ctr_marshal_keyvalues(meths->head, meths->size, &buffer, &blen, &bused);
    memcpy((*stream+*used), &bused, sizeof(ctr_size));
    *used += sizeof(ctr_size);
    memcpy((*stream+*used), buffer, bused);
    *used += bused;
    ctr_heap_free(buffer);
  }
}

void ctr_marshal_keyvalues(ctr_mapitem* head, ctr_size hlen, char** stream, ctr_size* stream_len, ctr_size* used) {
  if(!*stream) {
    *stream = ctr_heap_allocate(sizeof(char)*1024);
    *stream_len = 1024;
  }
  memcpy(*stream+*used, &hlen, sizeof(ctr_size));
  *used += sizeof(ctr_size);

  for(;hlen>0;hlen--) {
    if((*used) >= (*stream_len)) {
      *stream_len = (*used) * MARSH_BUFFER_ENLARGEMENT_FACTOR;
      *stream = ctr_heap_reallocate(*stream, *stream_len);
    }

    ctr_object *key = head->key, *value = head->value;
    if(head->value->info.type == CTR_OBJECT_TYPE_OTNATFUNC) continue;
    ctr_marshal_object(key, stream, stream_len, used);
    ctr_marshal_object(value, stream, stream_len, used);

    head = head->next;
  }
}

void ctr_marshal_store_link(ctr_object* obj, char** stream, ctr_size* stream_len, ctr_size* used) {
  if(obj == CtrStdObject)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Object;
  else if(obj == CtrStdBlock)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Block;
  else if(obj == CtrStdString)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_String;
  else if(obj == CtrStdNumber)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Number;
  else if(obj == CtrStdBool)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Bool;
  else if(obj == CtrStdConsole)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Console;
  else if(obj == CtrStdNil)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Nil;
  else if(obj == CtrStdGC)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_GC;
  else if(obj == CtrStdMap)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Map;
  else if(obj == CtrStdArray)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Array;
  else if(obj == CtrStdIter)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Iter;
  else if(obj == CtrStdFile)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_File;
  else if(obj == CtrStdSystem)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_System;
  else if(obj == CtrStdDice)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Dice;
  else if(obj == CtrStdCommand)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Command;
  else if(obj == CtrStdSlurp)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Slurp;
  else if(obj == CtrStdShell)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Shell;
  else if(obj == CtrStdClock)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Clock;
  else if(obj == CtrStdReflect)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Reflect;
  else if(obj == CtrStdReflect_cons)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Reflect_cons;
  else if(obj == CtrStdFiber)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Fiber;
  else if(obj == CtrStdThread)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Thread;
  else if(obj == CtrStdSymbol)
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_Symbol;
  else {
    *(*stream+((*used)++)) = CTR_MARSH_OBJECT_LINK_DESCRIBE;
    ctr_marshal_object(obj->link, stream, stream_len, used);
  }
}

ctr_object* ctr_marshal_resolve_linq(char* stream, ctr_size* stream_len, ctr_size* used) {
  char type = *(stream+((*used)++));
  switch(type) {
    case CTR_MARSH_OBJECT_LINK_Object: return CtrStdObject;
    case CTR_MARSH_OBJECT_LINK_Block: return CtrStdBlock;
    case CTR_MARSH_OBJECT_LINK_String: return CtrStdString;
    case CTR_MARSH_OBJECT_LINK_Number: return CtrStdNumber;
    case CTR_MARSH_OBJECT_LINK_Bool: return CtrStdBool;
    case CTR_MARSH_OBJECT_LINK_Console: return CtrStdConsole;
    case CTR_MARSH_OBJECT_LINK_Nil: return CtrStdNil;
    case CTR_MARSH_OBJECT_LINK_GC: return CtrStdGC;
    case CTR_MARSH_OBJECT_LINK_Map: return CtrStdMap;
    case CTR_MARSH_OBJECT_LINK_Array: return CtrStdArray;
    case CTR_MARSH_OBJECT_LINK_Iter: return CtrStdIter;
    case CTR_MARSH_OBJECT_LINK_File: return CtrStdFile;
    case CTR_MARSH_OBJECT_LINK_System: return CtrStdSystem;
    case CTR_MARSH_OBJECT_LINK_Dice: return CtrStdDice;
    case CTR_MARSH_OBJECT_LINK_Command: return CtrStdCommand;
    case CTR_MARSH_OBJECT_LINK_Slurp: return CtrStdSlurp;
    case CTR_MARSH_OBJECT_LINK_Shell: return CtrStdShell;
    case CTR_MARSH_OBJECT_LINK_Clock: return CtrStdClock;
    case CTR_MARSH_OBJECT_LINK_Reflect: return CtrStdReflect;
    case CTR_MARSH_OBJECT_LINK_Reflect_cons: return CtrStdReflect_cons;
    case CTR_MARSH_OBJECT_LINK_Fiber: return CtrStdFiber;
    case CTR_MARSH_OBJECT_LINK_Thread: return CtrStdThread;
    case CTR_MARSH_OBJECT_LINK_Symbol: return CtrStdSymbol;
    default:
    case CTR_MARSH_OBJECT_LINK_DESCRIBE: return NULL; //TODO;
  }
}

ctr_size ctr_marshal_object(ctr_object* obj, char** stream, ctr_size* stream_len /* in char* */, ctr_size* used) {//We will allocate stream if it's NULL, or use it as-is (will assume the size fits)
  if(!*stream) {
    *stream = ctr_heap_allocate(sizeof(char)*1024);
    *stream_len = 1024;
  }
  char* buf;
  size_t cplen = 0;
  char type_p = 0;
  int allocated = 0;
  ctr_size len = 0;
  switch(obj->info.type) {
    case CTR_OBJECT_TYPE_OTNIL: {
      type_p = CTR_MARSH_OTNIL;
    break;
    }
    case CTR_OBJECT_TYPE_OTBOOL: {
      len=1;
      char bf[2];
      type_p = CTR_MARSH_OTBOOL;
      sprintf(bf, "%c", obj->value.bvalue);
      buf = bf;
    break;
    }
    case CTR_OBJECT_TYPE_OTNUMBER: {
      len=sizeof(ctr_number);
      allocated = 1;
      buf = ctr_heap_allocate(len);
      type_p = CTR_MARSH_OTNUMBER;
      memcpy(buf, &(obj->value.nvalue), len);
    break;
    }
    case CTR_OBJECT_TYPE_OTSTRING: {
      len=obj->value.svalue->vlen;
      allocated = 1;
      buf = ctr_heap_allocate(len);
      type_p = CTR_MARSH_OTSTRING;
      cplen = obj->value.svalue->vlen;
      memcpy(buf, obj->value.svalue->value, sizeof(char)*obj->value.svalue->vlen);
    break;
    }
    case CTR_OBJECT_TYPE_OTARRAY: {
      ctr_size bused = len, blen = 64;
      allocated = 1;
      buf = ctr_heap_allocate(64);
      type_p = CTR_MARSH_OTARRAY;
      cplen = obj->value.avalue->head-obj->value.avalue->tail;
      for(ctr_size ie=obj->value.avalue->tail; ie<obj->value.avalue->head; ie++) {
        (void)ctr_marshal_object(obj->value.avalue->elements[ie], &buf, &blen, &bused);
      }
      len = bused;
    break;
    }
    case CTR_OBJECT_TYPE_OTBLOCK: { //oh boi here we go HACK: this.
      ctr_tnode* block = obj->value.block;
      ctr_marshal_ast(block, stream, stream_len, used);
    break;
    }
    case CTR_OBJECT_TYPE_OTOBJECT: { //OBJECT <link description> <own_properties>
      type_p = CTR_MARSH_OTOBJECT;
    break;
    }
    case CTR_OBJECT_TYPE_OTNATFUNC: {

    // break;
    }
    case CTR_OBJECT_TYPE_OTMISC: {

    // break;
    }
    case CTR_OBJECT_TYPE_OTEX: {

    // break;
    }
    default: {
      len = 0;
      abort();
    }
  }
  if((*used) + len + (type_p != 0) >= (*stream_len)) {
    *stream_len = (*used + len + type_p!=0) * MARSH_BUFFER_ENLARGEMENT_FACTOR;
    *stream = ctr_heap_reallocate(*stream, *stream_len);
  }
  if(type_p) *(*stream+((*used)++)) = type_p;
  // if(type_p == CTR_MARSH_OTOBJECT)
    // ctr_marshal_store_link(obj, stream, stream_len, used);
  if(cplen) {
    memcpy(*stream+*used, &cplen, sizeof(ctr_size));
    *used += sizeof(ctr_size);
  }
  if(len)
    memcpy((*stream)+(*used), buf, len);
  (*used) += len;
  ctr_marshal_meta_object(obj, stream, stream_len, used);
  if(allocated) ctr_heap_free(buf);
  return *used;
}

ctr_map* ctr_unmarshal_keyvalues(char* stream, ctr_size* avail, ctr_size* consumed) {
  ctr_size len = *(ctr_size*)(stream+(*consumed)); //count
  *consumed+=sizeof(ctr_size);
  ctr_map* map = ctr_heap_allocate(sizeof(*map));
  ctr_mapitem* head = ctr_heap_allocate(sizeof(*head));
  map->size = len;
  map->head = head;
  for(;len>0;len--) {
    ctr_object *key = ctr_unmarshal_object(stream, avail, consumed), *value = ctr_unmarshal_object(stream, avail, consumed);
    if(!key||!value) {
      printf("NULL encountered in deserialization of keyvalue: [%lu]%p:%p (around '", len, key, value);
      fwrite(stream, 1, *avail, stdout);
      puts("')");
    //  map->size--; continue;
    }
    head->key = key;
    head->value = value;
    head->hashKey = ctr_internal_index_hash(key);
    if(len>1) {
      ctr_mapitem* nhead = ctr_heap_allocate(sizeof(*nhead));
      head->next = nhead;
      nhead->prev = head;
      head = nhead;
    }
  }
  return map;
}

void ctr_unmarshal_meta_object_into(ctr_object* obj, char* stream, ctr_size* avail, ctr_size* consumed) {
  char next_in_stream = *(stream+((*consumed)++));
  ctr_map *props = NULL, *meths = NULL;
  ctr_mapitem *proplast, *methlast;
  switch(next_in_stream) {
    case CTR_MARSH_META_PROP: {
      // ctr_size len = *(ctr_size*)(stream+(*consumed)); //count
      *consumed+=sizeof(ctr_size);
      props = ctr_unmarshal_keyvalues(stream, avail, consumed);
    break;
    }
    case CTR_MARSH_META_METH: {
      *consumed+=sizeof(ctr_size);
      meths = ctr_unmarshal_keyvalues(stream, avail, consumed);
    break;
    }
    default: (*consumed)--; return;
  }
  if(!obj) return; //read the metadata, but skip adding them
  if(meths) {
    methlast = get_last_item(meths);
    if(obj->methods) {
      if(obj->methods->size > 0) {
        obj->methods->size += meths->size;
        methlast->next = obj->methods->head;
        obj->methods->head->prev = methlast;
        obj->methods->head = meths->head;
      } else {
        obj->methods->size = meths->size;
        obj->methods->head = meths->head;
      }
    } else {
      obj->methods = meths;
    }
    ctr_heap_free(meths);
  }
  if(props) {
    proplast = get_last_item(props);
    if(obj->properties) {
      if(obj->properties->size > 0) {
        obj->properties->size += props->size;
        proplast->next = obj->properties->head;
        obj->properties->head->prev = proplast;
        obj->properties->head = props->head;
      } else {
        obj->properties->size = props->size;
        obj->properties->head = props->head;
      }
    } else {
      obj->properties = props;
    }
    ctr_heap_free(props);
  }
}

ctr_object* ctr_unmarshal_object(char* stream, ctr_size* avail, ctr_size* consumed) {
  char type = *(stream+*consumed);
  (*consumed)++;
  ctr_object* umarshed = NULL;
  switch(type) {
    case CTR_MARSH_OTNIL: {
      (*consumed)+=1;
      umarshed = CtrStdNil;
      break;
    }
    case CTR_MARSH_OTBOOL: {
      umarshed = ctr_build_bool(*(stream+((*consumed)++)) == 1);
      break;
    }
    case CTR_MARSH_OTNUMBER: {
      ctr_object* ret = ctr_build_number_from_float(0);
      ret->value.nvalue = *(ctr_number*)(stream+(*consumed));
      (*consumed)+=sizeof(ctr_number);
      umarshed = ret;
    break;
    }
    case CTR_MARSH_OTSTRING: {
      ctr_size len = *(ctr_size*)(stream+(*consumed));
      (*consumed) += sizeof(ctr_size);
      umarshed = ctr_build_string(stream+(*consumed), len);
      *consumed += len;
      break;
    }
    case CTR_MARSH_OTARRAY: {
      ctr_size len = *(ctr_size*)(stream+(*consumed)); //count
      ctr_object* array = ctr_array_new(CtrStdArray, NULL);
      ctr_argument* arg = ctr_heap_allocate(sizeof(*arg));
      (*consumed) += sizeof(len);
      for(ctr_size ei=0;ei<len;ei++) {
        arg->object = ctr_unmarshal_object(stream, avail, consumed);
        ctr_array_push(array, arg);
      }
      ctr_heap_free(arg);
      umarshed = array;
    break;
    }
    case CTR_MARSH_OTBLOCKL:
    case CTR_MARSH_OTBLOCKNL: {
      (*consumed)--;
      ctr_object* block = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
      block->value.block = ctr_unmarshal_ast(stream, *avail, consumed);
      block->link = CtrStdBlock;
      umarshed = block;
    break;
    }
    case CTR_MARSH_OTOBJECT: {
      umarshed = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
    break;
    }
    case CTR_MARSH_OTNATFUNC:
    case CTR_MARSH_OTMISC:
    case CTR_MARSH_OTEX:

    case CTR_MARSH_META_METH:
    case CTR_MARSH_META_PROP:
    case CTR_MARSH_NMAP: //what is it doing here?
    default: {
      putchar(type);
      abort();
    }
  }
  ctr_unmarshal_meta_object_into(umarshed, stream, avail, consumed);
  // if(umarshed&&umarshed->info.type == CTR_OBJECT_TYPE_OTOBJECT) umarshed->link = ctr_marshal_resolve_linq(stream, avail, consumed);
  return umarshed;
}

ctr_object* ctr_marshal(ctr_object* object) {
  ctr_size slen = 1024, used=0;
  char* buf = ctr_heap_allocate(slen);
  ctr_marshal_object(object, &buf, &slen, &used);
  if(!used) {
    return 0;
  }
  // char* bf = ctr_heap_allocate(sizeof(char)*len+1);
  ctr_object* ret = ctr_build_string(buf, used);
  return ret;
}

ctr_object* ctr_unmarshal(char* object, size_t len) {
  ctr_size c = 0, l = len;
  return ctr_unmarshal_object(object, &l, &c);
}


ctr_object* ctr_reflect_marshal(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* ret = ctr_marshal(argumentList->object);
  return ret?ret:ctr_build_nil();
}

ctr_object* ctr_reflect_unmarshal(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* ret = ctr_unmarshal(argumentList->object->value.svalue->value, argumentList->object->value.svalue->vlen);
  return ret?ret:ctr_build_nil();
}
