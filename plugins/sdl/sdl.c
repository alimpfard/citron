#include "../../citron.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL.h"

#define NEW true

/*
 * SDL interface
 */
ctr_object* ctr_sdl_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_init(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_set_prop(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_quit(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_rect_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_rect_alterx(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_rect_altery(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_rect_alterw(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_rect_alterh(ctr_object* myself, ctr_argument* argumentList);
/*
 * Color interface
 */
ctr_object* ctr_sdl_color_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_color_set(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_color_get(ctr_object* myself, ctr_argument* argumentList);
/*
 * SDL Surface interface
 */
ctr_object* ctr_sdl_surface_loadBMP(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_free_surface(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_display_format(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_lock(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_unlock(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_set_pixel(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_blit(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_blit_loc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_fill(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_update(ctr_object* myself, ctr_argument* argumentList);
/*
 * SDL Event interface
 */
SDL_Event* events;
int events_count = 0;
int events_current = 0;
ctr_object* ctr_sdl_event_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_poll_event(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_button_(ctr_object* myself);
ctr_object* ctr_sdl_event_get_param(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_key_(ctr_object* myself);
ctr_object* ctr_sdl_event_key_get_keysym(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_key_get_repeat(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_key_get_state(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_key_get_timestamp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_key_get_windowID(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_event_motion_(ctr_object* myself);
ctr_object* ctr_sdl_event_text_(ctr_object* myself);
ctr_object* ctr_sdl_event_wheel_(ctr_object* myself);
/*
 * SDL Event Type CTR Impl
 */
ctr_object* ctr_sdl_event_types_make(ctr_object* myself, int type, char* name);

// Helper funcs
SDL_Surface* get_sdl_surface_ptr(ctr_object* myself) {
  intptr_t surface = (ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring("surfacePtr"),
    CTR_CATEGORY_PRIVATE_PROPERTY
  )->value.nvalue);
  return (SDL_Surface*)surface;
}
SDL_Rect* get_sdl_rect_ptr(ctr_object* myself) {
  ctr_object* rectobj = (ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring("rectPtr"),
    CTR_CATEGORY_PRIVATE_PROPERTY
  ));
  if (rectobj == NULL) return NULL;
  return (SDL_Rect*)(intptr_t)(rectobj->value.nvalue);
}

int get_sdl_event_index(ctr_object* myself) {
  ctr_object* index = ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring("eventId"),
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  if (index == NULL) return -2;
  int i = index->value.nvalue;
  if(i >= events_count) return -1;
  return i;
}

int ctr_heap_allocate_sdl_event() {
  int curr = -1;
  if (events_current < events_count - 2) curr = events_current++;
  else if(events_count > 0){
    events = ctr_heap_reallocate(events, sizeof(SDL_Event) * events_count * 2);
    curr = events_current++;
    events_count *= 2;
  }
  else {
    events = ctr_heap_allocate(sizeof(SDL_Event) * 2);
    curr = events_current++;
    events_count = 2;
  }
  if (curr == -1) {printf("Current id is -1\n");return (-1);}
  return curr;
}

void ctr_sdl_quit_atexit() {
  ctr_heap_free(events);
  SDL_Quit();
}

/**
 *  SDL main entry point
 *
 *  SDL main functions
 */

ctr_object* ctr_sdl_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = myself;
  ctr_internal_object_set_property(
   instance,
   ctr_build_string_from_cstring( "windowHeight" ),
   ctr_build_number_from_float(480),
   CTR_CATEGORY_PRIVATE_PROPERTY
 );
 ctr_internal_object_set_property(
   instance,
   ctr_build_string_from_cstring( "windowWidth" ),
   ctr_build_number_from_float(640),
   CTR_CATEGORY_PRIVATE_PROPERTY
 );
 ctr_internal_object_set_property(
   instance,
   ctr_build_string_from_cstring( "windowName" ),
   ctr_build_string_from_cstring( "Citron SDL Window" ),
   CTR_CATEGORY_PRIVATE_PROPERTY
 );
 return instance;
}

ctr_object* ctr_sdl_set_prop(ctr_object* myself, ctr_argument* argumentList) {
  ctr_internal_object_set_property(myself, ctr_internal_cast2string(argumentList->object), argumentList->next->object, CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}

ctr_object* ctr_sdl_init(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* cap = ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring("windowName"),
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  char* caption = ctr_heap_allocate_cstring (cap);
  int w, h;
  w = ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring("windowWidth"),
    CTR_CATEGORY_PRIVATE_PROPERTY
  )->value.nvalue;
  h = ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring("windowHeight"),
    CTR_CATEGORY_PRIVATE_PROPERTY
  )->value.nvalue;
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    char* err = "Unable to initialize SDL: ";
    strcat(err, SDL_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
    ctr_heap_free(caption);
    return CtrStdFlow;
  }
  atexit(ctr_sdl_quit_atexit);
  SDL_WM_SetCaption (caption, caption);
  SDL_Surface* window = SDL_SetVideoMode(w, h, 0, 0);
  if (window == NULL) {
    char* err = "Unable to set Video Mode: ";
    strcat(err, SDL_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
    ctr_heap_free(caption);
    return CtrStdFlow;
  }
  ctr_heap_free(caption);
  ctr_internal_object_set_property(
    myself,
    ctr_build_string_from_cstring( "surfacePtr" ),
    ctr_build_number_from_float((intptr_t)window),
    CTR_CATEGORY_PRIVATE_PROPERTY
  );
  return myself;
}
ctr_object* ctr_sdl_quit(ctr_object* myself, ctr_argument* argumentList) {
  ctr_heap_free(events);
  SDL_Quit();
  return ctr_build_nil();
}

ctr_object* ctr_sdl_rect_make(ctr_object* myself, ctr_argument* argumentList) {
  int x, y, w, h;
  x = argumentList->object->value.nvalue;
  y = argumentList->next->object->value.nvalue;
  w = argumentList->next->next->object->value.nvalue;
  h = argumentList->next->next->next->object->value.nvalue;
  SDL_Rect* Rect = ctr_heap_allocate_tracked (sizeof(SDL_Rect) ); //We're gonna keep this 'till the end.
  Rect->x = x;
  Rect->y = y;
  Rect->w = w;
  Rect->h = h;
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = myself;
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("rectPtr"), ctr_build_number_from_float((intptr_t)(Rect)), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_create_func(instance, ctr_build_string_from_cstring( "setX:" ), &ctr_sdl_rect_alterx);
  ctr_internal_create_func(instance, ctr_build_string_from_cstring( "setY:" ), &ctr_sdl_rect_altery);
  ctr_internal_create_func(instance, ctr_build_string_from_cstring( "setW:" ), &ctr_sdl_rect_alterw);
  ctr_internal_create_func(instance, ctr_build_string_from_cstring( "setH:" ), &ctr_sdl_rect_alterh);
  return instance;
}

ctr_object* ctr_sdl_rect_alterx(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("rectPtr"), ctr_build_number_from_float((intptr_t)rect), CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}
ctr_object* ctr_sdl_rect_altery(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->y = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("rectPtr"), ctr_build_number_from_float((intptr_t)rect), CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}
ctr_object* ctr_sdl_rect_alterw(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->w = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("rectPtr"), ctr_build_number_from_float((intptr_t)rect), CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}
ctr_object* ctr_sdl_rect_alterh(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->h = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("rectPtr"), ctr_build_number_from_float((intptr_t)rect), CTR_CATEGORY_PRIVATE_PROPERTY);
  return myself;
}

#ifdef NEW
/**
 * SDL Color Interface
 *
 *
 * Color handlers
 */

 ctr_object* ctr_sdl_color_make(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
   instance->link = myself;
   if(argumentList == NULL) return instance;
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("red"), (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("green"), (argumentList->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("blue"), (argumentList->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("alpha"), (argumentList->next->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   return instance;
 }

 ctr_object* ctr_sdl_color_set(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object *prop = ctr_internal_cast2string(argumentList->object), *val = ctr_internal_cast2number(argumentList->next->object);
   if (prop==NULL||val==NULL) {
     CtrStdFlow = prop==NULL?ctr_build_string_from_cstring("Propery name cannot be Nil."):ctr_build_string_from_cstring("Value must be a number.");
     return CtrStdFlow;
   }
   ctr_internal_object_set_property(myself, prop, val, CTR_CATEGORY_PRIVATE_PROPERTY);
   return myself;
 }
 ctr_object* ctr_sdl_color_get(ctr_object* myself, ctr_argument* argumentList) {
   return ctr_internal_object_find_property(myself, ctr_internal_cast2string(argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
 }
 #endif

/**
 *  SDL surface entry point
 *
 *  SDL surface handler functions
 */

ctr_object* ctr_sdl_surface_loadBMP(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = myself;
  char* loc = ctr_heap_allocate_cstring (argumentList->object);
  SDL_Surface* temp = SDL_LoadBMP(loc);
  temp = SDL_DisplayFormat(temp);
  ctr_internal_object_set_property(
   instance,
   ctr_build_string_from_cstring( "surfacePtr" ),
   ctr_build_number_from_float((intptr_t)temp),
   CTR_CATEGORY_PRIVATE_PROPERTY
 );
 ctr_heap_free(loc);
 return instance;
}

ctr_object* ctr_sdl_surface_free_surface(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_FreeSurface (surface);
  return ctr_build_nil();
}

ctr_object* ctr_sdl_surface_display_format(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = CtrStdObject;
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_Surface* temp = SDL_DisplayFormat(surface);
  ctr_internal_object_set_property(
   instance,
   ctr_build_string_from_cstring( "surfacePtr" ),
   ctr_build_number_from_float((intptr_t)temp),
   CTR_CATEGORY_PRIVATE_PROPERTY
  );
  return instance;
}

ctr_object* ctr_sdl_surface_blit(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_Surface* what = get_sdl_surface_ptr(argumentList->object);
  SDL_BlitSurface(what, NULL, on, NULL);
  return myself;
}

ctr_object* ctr_sdl_surface_blit_loc(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_Surface* what = get_sdl_surface_ptr(argumentList->object);
  SDL_Rect *from, *to;
  from = get_sdl_rect_ptr(argumentList->next->object);
  to = get_sdl_rect_ptr(argumentList->next->next->object);
  SDL_BlitSurface(what, from, on, to);
  return myself;
}

ctr_object* ctr_sdl_surface_fill(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_Rect* rect = get_sdl_rect_ptr(argumentList->object);
  #ifdef NEW
  //TODO: Get color interface
  ctr_object* color_ = argumentList->next->object;
  int r,g,b;
  r = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  g = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  b = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  uint32_t color = SDL_MapRGB(surface->format, r, g, b);
  #else
  uint32_t color = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  #endif
  SDL_FillRect(surface, rect, color);
  return myself;
}

ctr_object* ctr_sdl_surface_update(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_UpdateRect(on, 0, 0, 0, 0);
  return myself;
}
#ifdef NEWSURF
ctr_object* ctr_sdl_surface_lock(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);

}
ctr_object* ctr_sdl_surface_unlock(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);

}
#endif
#ifdef NEW
void ctr_sdl_put_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}
ctr_object* ctr_sdl_surface_set_pixel(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  int x, y;
  x = argumentList->object->value.nvalue;
  y = argumentList->next->object->value.nvalue;
  ctr_object* color_ = argumentList->next->next->object;
  int r,g,b;
  r = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  g = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  b = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  uint32_t color = SDL_MapRGB(surface->format, r, g, b);
  ctr_sdl_put_pixel(surface, x, y, color);
  return myself;
}
#endif
/**
 * Event Interface
 *
 */
ctr_object* ctr_sdl_event_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = myself;
  int event_id = ctr_heap_allocate_sdl_event();
  if (event_id == -1) {CtrStdFlow = ctr_build_string_from_cstring("<New> Could not find event."); return CtrStdFlow;}
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("eventId"), ctr_build_number_from_float(event_id), CTR_CATEGORY_PRIVATE_PROPERTY);
  return instance;
}

ctr_object* ctr_sdl_event_poll_event(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* cb = argumentList->object;
  ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  int id = get_sdl_event_index(myself);
  if (id <= -1) {CtrStdFlow = ctr_build_string_from_cstring("<Poll> Could not find event."); return CtrStdFlow;}
  if(SDL_PollEvent(events+id)) {
    arguments->object = myself;
    ctr_block_run(cb, arguments, NULL);
  }
  ctr_heap_free( arguments );
  return ctr_build_nil();
}
ctr_object* ctr_sdl_event_get_param(ctr_object* myself, ctr_argument* argumentList) {
  char* opt = ctr_heap_allocate_cstring(argumentList->object);
  ctr_object* instance;
  if (strcmp(opt, "key") == 0) {
    instance = ctr_sdl_event_key_(myself);
  }
  // else if (strncmp(opt, "motion", 6))
  //   instance = ctr_sdl_event_motion_(myself);
  // else if (strncmp(opt, "button", 6))
  //   instance = ctr_sdl_event_button_(myself);
  // else if (strncmp(opt, "text", 4))
  //   instance = ctr_sdl_event_text_(myself);
  // else if (strncmp(opt, "wheel", 5))
  //   instance = ctr_sdl_event_wheel_(myself);
  else
    instance = CtrStdNil;
  ctr_heap_free(opt);
  return instance;
}
ctr_object* ctr_sdl_event_key_   (ctr_object* myself) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  //ctr_internal_create_func(instance, ctr_build_string_from_cstring("timestamp"), &ctr_sdl_event_key_get_timestamp);
  //ctr_internal_create_func(instance, ctr_build_string_from_cstring("windowID"), &ctr_sdl_event_key_get_windowID);
  ctr_internal_create_func(instance, ctr_build_string_from_cstring("state"), &ctr_sdl_event_key_get_state);
  //ctr_internal_create_func(instance, ctr_build_string_from_cstring("repeat"), &ctr_sdl_event_key_get_repeat);
  ctr_internal_create_func(instance, ctr_build_string_from_cstring("keysym"), &ctr_sdl_event_key_get_keysym);
  return instance;
}
// ctr_object* ctr_sdl_event_key_get_timestamp (ctr_object* myself, ctr_argument* argumentList) {
//   ctr_object* instance = ctr_build_number_from_float(event.key.timestamp);
//   return instance;
// }
// ctr_object* ctr_sdl_event_key_get_windowID (ctr_object* myself, ctr_argument* argumentList) {
//   ctr_object* instance = ctr_build_number_from_float(event.key.windowID);
//   return instance;
// }
ctr_object* ctr_sdl_event_key_get_state (ctr_object* myself, ctr_argument* argumentList) {
  int id = get_sdl_event_index(myself);
  if (id <= -1) {CtrStdFlow = ctr_build_string_from_cstring("<KeyState> Could not allocate event."); return CtrStdFlow;}
  ctr_object* instance = ctr_build_number_from_float(events[id].key.state);
  return instance;
}
// ctr_object* ctr_sdl_event_key_get_repeat (ctr_object* myself, ctr_argument* argumentList) {
//   ctr_object* instance = ctr_build_number_from_float(event.key.repeat);
//   return instance;
// }
ctr_object* ctr_sdl_event_key_get_keysym (ctr_object* myself, ctr_argument* argumentList) {
  int id = get_sdl_event_index(myself);
  printf("%d\n", id);
  if (id <= -1) {CtrStdFlow = ctr_build_string_from_cstring("<KeySym> Could not find event."); return CtrStdFlow;}
  ctr_object* instance = ctr_build_string_from_cstring(SDL_GetKeyName(events[id].key.keysym.sym));
  return instance;
}
// ctr_object* ctr_sdl_event_motion_(ctr_object* myself) {
//   ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   return instance;
// }
// ctr_object* ctr_sdl_event_button_(ctr_object* myself) {
//   ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   return instance;
// }
// ctr_object* ctr_sdl_event_text_  (ctr_object* myself) {
//   ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(""), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   return instance;
// }
// ctr_object* ctr_sdl_event_wheel_ (ctr_object* myself) {
//   ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(/**/), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(/**/), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring(/**/), /**/, CTR_CATEGORY_PRIVATE_PROPERTY);
//   return instance;
// }
/*
 * SDL Event Type
 */

ctr_object* ctr_sdl_event_type(ctr_object* myself, ctr_argument* argumentList) {
  int id = get_sdl_event_index(myself);
  if (id <= -1) {CtrStdFlow = ctr_build_string_from_cstring("<Type> Could not find event."); return CtrStdFlow;}
  return ctr_build_number_from_float(events[id].type);
}

ctr_object* ctr_sdl_event_type_compare(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* other = ctr_internal_cast2number(argumentList->object);
  int id = get_sdl_event_index(myself);
  if (id <= -1) {CtrStdFlow = ctr_build_string_from_cstring("<Compare> Could not find event."); return CtrStdFlow;}
  return ctr_build_bool(other!=NULL && (other->value.nvalue) == (double)(events[id].type));
}


void begin() {
  ctr_object* sdlObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_object* evtO = ctr_sdl_event_make(CtrStdObject, NULL);
  #ifdef NEW
  ctr_object* color = ctr_sdl_color_make(CtrStdObject, NULL);
  #endif
	sdlObject->link = CtrStdObject;
  //sdl, surface
	ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "new" ), &ctr_sdl_make );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "init" ), &ctr_sdl_init );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "quit" ), &ctr_sdl_quit );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "loadBMP:" ), &ctr_sdl_surface_loadBMP );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "blit:" ), &ctr_sdl_surface_blit );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "blit:fromRect:toRect:" ), &ctr_sdl_surface_blit_loc );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "fillRect:withColor:" ), &ctr_sdl_surface_fill );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "update" ), &ctr_sdl_surface_update );
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "set:to:" ), &ctr_sdl_set_prop);
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "setPixelWithX:andY:toColor:" ), &ctr_sdl_surface_set_pixel);
  ctr_internal_create_func(sdlObject, ctr_build_string_from_cstring( "newRectWithX:andY:andW:andH:" ), &ctr_sdl_rect_make );

  //event
  ctr_internal_create_func(evtO, ctr_build_string_from_cstring( "new" ), &ctr_sdl_event_make );
  ctr_internal_create_func(evtO, ctr_build_string_from_cstring( "poll:" ), &ctr_sdl_event_poll_event );
  ctr_internal_create_func(evtO, ctr_build_string_from_cstring( "toNumber" ), &ctr_sdl_event_type);
  ctr_internal_create_func(evtO, ctr_build_string_from_cstring( "equals:" ), &ctr_sdl_event_type_compare);
  ctr_internal_create_func(evtO, ctr_build_string_from_cstring( "get:" ), &ctr_sdl_event_get_param);
  #ifdef NEW
  //Color
  ctr_internal_create_func(color, ctr_build_string_from_cstring( "red:green:blue:alpha:" ), &ctr_sdl_color_make);
  ctr_internal_create_func(color, ctr_build_string_from_cstring( "get:" ), &ctr_sdl_color_get);
  ctr_internal_create_func(color, ctr_build_string_from_cstring( "set:to:" ), &ctr_sdl_color_set);

  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Color" ), color, 0);
  #endif
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDLEvent" ), evtO, 0);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDL" ), sdlObject, 0);

  //types
  ctr_argument* ArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
	ArgumentList->next = NULL;
  ArgumentList->object = ctr_build_string_from_cstring("extensions/sdl.ctr");
  ctr_object* ext = ctr_file_new(CtrStdFile, ArgumentList);
  ctr_file_include(ext, NULL);
  ctr_heap_free(ArgumentList);
}
