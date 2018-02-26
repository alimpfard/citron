#include "../../citron.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>

#define NEW 1

#define CTR_SDL_TYPE_BASE 2

enum ctr_sdl {
  CTR_SDL_TYPE_SURFACE = CTR_SDL_TYPE_BASE,
  CTR_SDL_TYPE_RECT,
  CTR_SDL_TYPE_EVT
};

enum ctr_sdl_event_type {
  CTR_SDL_EVT_KEY
};

struct ctr_sdl_event {
  enum ctr_sdl_event_type type;
  void* ptr;
};

typedef enum ctr_sdl ctr_sdl_t;
typedef enum ctr_sdl_event_type ctr_sdl_evt_t;
typedef struct ctr_sdl_event ctr_sdl_evt;


ctr_object* CtrStdSdl;
ctr_object* CtrStdSdl_surface;
ctr_object* CtrStdSdl_rect;
ctr_object* CtrStdSdl_evt;
ctr_object* CtrStdColor;

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
ctr_object* ctr_sdl_surface_loadImage(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_free_surface(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_display_format(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_lock(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_unlock(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_set_pixel(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_blit(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_blit_loc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_fill(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_get_clip_rect(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_scale(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_scale_s(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_set_color_key(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_surface_update(ctr_object* myself, ctr_argument* argumentList);
/*
 * SDL Event interface
 */
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
ctr_object* ctr_sdl_evt_init(ctr_object* object, ctr_sdl_evt_t type) {
  object->value.rvalue->ptr = ctr_heap_allocate_tracked(sizeof(ctr_sdl_evt));
  ((ctr_sdl_evt*)(object->value.rvalue->ptr))->type = type;
  return object;
}
ctr_object* ctr_sdl_create_container_of_type(ctr_sdl_t type) {
  ctr_object* object = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
  object->value.rvalue = ctr_heap_allocate_tracked(sizeof(ctr_resource));
  object->value.rvalue->type = (int)type;
  if (type == CTR_SDL_TYPE_EVT) ctr_sdl_evt_init(object,-1);
  return object;
}

SDL_Surface* get_sdl_surface_ptr(ctr_object* myself) {
  void* surface = myself->value.rvalue->ptr;
  return (SDL_Surface*)surface;
}
SDL_Rect* get_sdl_rect_ptr(ctr_object* myself) {
  void* rect = myself->value.rvalue->ptr;
  return (SDL_Rect*)rect;
}
ctr_sdl_evt* get_sdl_event_ptr(ctr_object* myself) {
  void* evt = myself->value.rvalue->ptr;
  return (ctr_sdl_evt*)evt;
}

void ctr_sdl_quit_atexit() {
  IMG_Quit();
  SDL_Quit();
}

/**
 *  SDL main entry point
 *
 *  SDL main functions
 */

ctr_object* ctr_sdl_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  instance->link = CtrStdSdl;
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
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    char* err = "Unable to initialize SDL: ";
    err = strcat(err, SDL_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
    ctr_heap_free(caption);
    return CtrStdFlow;
  }
  SDL_WM_SetCaption (caption, caption);
  SDL_Surface* window = SDL_SetVideoMode(w, h, 0, 0);
  if (window == NULL) {
    char* err = "Unable to set Video Mode: ";
    err = strcat(err, SDL_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
    ctr_heap_free(caption);
    return CtrStdFlow;
  }

  // load support for the JPG and PNG image formats
  int flags=IMG_INIT_JPG|IMG_INIT_PNG;
  int initted=IMG_Init(flags);
  if((initted&flags) != flags) {
    char* err = strcat("IMG failed to initialize: ", IMG_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
    return CtrStdFlow;
  }
  ctr_heap_free(caption);
  myself->link = CtrStdSdl_surface;
  myself->value.rvalue->ptr = (void*)window;
  return myself;
}
ctr_object* ctr_sdl_quit(ctr_object* myself, ctr_argument* argumentList) {
  IMG_Quit();
  SDL_Quit();
  return CtrStdNil;
}

SDL_Rect* ctr_sdl_rect_make_nat(int x, int y, int w, int h) {
  SDL_Rect* Rect = ctr_heap_allocate (sizeof(SDL_Rect) );
  Rect->x = x;
  Rect->y = y;
  Rect->w = w;
  Rect->h = h;
  return Rect;
}

ctr_object* ctr_sdl_rect_make(ctr_object* myself, ctr_argument* argumentList) {
  int x, y, w, h;
  x = argumentList->object->value.nvalue;
  y = argumentList->next->object->value.nvalue;
  w = argumentList->next->next->object->value.nvalue;
  h = argumentList->next->next->next->object->value.nvalue;
  SDL_Rect* Rect = ctr_sdl_rect_make_nat(x,y,w,h);
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_RECT);
  instance->link = CtrStdSdl_rect;
  instance->value.rvalue->ptr = (void*)Rect;
  return instance;
}

ctr_object* ctr_sdl_rect_alterx(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  return myself;
}
ctr_object* ctr_sdl_rect_altery(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->y = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  return myself;
}
ctr_object* ctr_sdl_rect_alterw(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->w = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  return myself;
}
ctr_object* ctr_sdl_rect_alterh(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  rect->h = ctr_internal_cast2number(argumentList->object)->value.nvalue;
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
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("red"),  (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("green"),(argumentList->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("blue"), (argumentList->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("alpha"),(argumentList->next->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   return instance;
 }

 ctr_object* ctr_sdl_color_make_rgba(ctr_object* myself, ctr_argument* argumentList) {
   if(argumentList->object->info.type != CTR_OBJECT_TYPE_OTARRAY) goto err_not_arr;
   if(ctr_array_count(argumentList->object, NULL)->value.nvalue != 4) goto err_not_enough;
   ctr_object *r,*g,*b,*a;
   r = ctr_array_head(argumentList->object, NULL);
   g = ctr_invoke_variadic(argumentList->object, &ctr_array_get, 1, ctr_build_number_from_float(1));
   b = ctr_invoke_variadic(argumentList->object, &ctr_array_get, 1, ctr_build_number_from_float(2));
   a = ctr_invoke_variadic(argumentList->object, &ctr_array_get, 1, ctr_build_number_from_float(3));

   return ctr_invoke_variadic(myself, &ctr_sdl_color_make, 4, r,g,b,a);

   err_not_arr:
   CtrStdFlow = ctr_build_string_from_cstring("Expected an array of RGBA values");
   return myself;
   err_not_enough:
   CtrStdFlow = ctr_build_string_from_cstring("Expected an array of 4 values for RGBA");
   return myself;
 }
 ctr_object* ctr_sdl_color_set_rgb(ctr_object* myself, ctr_argument* argumentList) {
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("red"), (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("green"), (argumentList->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("blue"), (argumentList->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   if(argumentList->next->next->next->object) ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("alpha"), (argumentList->next->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   return myself;
 }
 ctr_object* ctr_sdl_color_make_rgb(ctr_object* myself, ctr_argument* argumentList) {
   if(argumentList->object->info.type != CTR_OBJECT_TYPE_OTARRAY) goto err_not_arr;
   if(ctr_array_count(argumentList->object, NULL)->value.nvalue != 3) goto err_not_enough;
   ctr_object *r,*g,*b;
   r = ctr_array_head(argumentList->object, NULL);
   g = ctr_invoke_variadic(argumentList->object, &ctr_array_get, 1, ctr_build_number_from_float(1));
   b = ctr_invoke_variadic(argumentList->object, &ctr_array_get, 1, ctr_build_number_from_float(2));

   return ctr_invoke_variadic(myself, &ctr_sdl_color_make, 4, r,g,b,ctr_build_number_from_float(255));

   err_not_arr:
   CtrStdFlow = ctr_build_string_from_cstring("Expected an array of RGB values");
   return myself;
   err_not_enough:
   CtrStdFlow = ctr_build_string_from_cstring("Expected an array of 3 values for RGB");
   return myself;
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
 ctr_object* ctr_sdl_color_to_string(ctr_object* myself, ctr_argument* argumentList) {
   if(myself == CtrStdColor) return ctr_build_string_from_cstring("RGBA[null,null,null,null]");
   char str[512];

   uint8_t r,g,b,a;
   r = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
   g = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
   b = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
   a = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring("alpha"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;

   sprintf(str, "RGBA[%i,%i,%i,%i]", r,g,b,a);
   return ctr_build_string_from_cstring(str);
 }
 #endif

/**
 *  SDL surface entry point
 *
 *  SDL surface handler functions
 */

ctr_object* ctr_sdl_surface_loadImage(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  instance->link = CtrStdSdl_surface;
  char* loc = ctr_heap_allocate_cstring (argumentList->object);
  if(access(loc, F_OK) == -1) {
    CtrStdFlow = ctr_build_string_from_cstring("specified file does not exist or cannot be accessed.");
    ctr_heap_free(loc);
    return CtrStdExit;
  }
  SDL_Surface* temp = IMG_Load(loc);
  SDL_Surface* tempcompat = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  ctr_heap_free(loc);
  instance->value.rvalue->ptr = tempcompat;
  return instance;
}

ctr_object* ctr_sdl_surface_free_surface(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_FreeSurface (surface);
  return ctr_build_nil();
}

ctr_object* ctr_sdl_surface_display_format(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = CtrStdSdl_surface;
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_Surface* temp = SDL_DisplayFormat(surface);
  instance->value.rvalue->ptr = (void*)temp;
  return instance;
}

ctr_object* ctr_sdl_surface_blit(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_Surface* what = get_sdl_surface_ptr(argumentList->object);
  // printf("blit\n");
  if(SDL_BlitSurface(what, NULL, on, NULL) < 0) {
    char* err;
    err = strcat("Blit failed: ", SDL_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
    return myself;
  }
  return myself;
}

ctr_object* ctr_sdl_surface_blit_loc(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_Surface* what = get_sdl_surface_ptr(argumentList->object);
  SDL_Rect *from, *to;
  from = argumentList->next->object == CtrStdNil ? NULL : get_sdl_rect_ptr(argumentList->next->object);
  to = argumentList->next->next->object == CtrStdNil ? NULL : get_sdl_rect_ptr(argumentList->next->next->object);
  SDL_BlitSurface(what, from, on, to);
  return myself;
}

ctr_object* ctr_sdl_surface_fill(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_Rect* rect = argumentList->object!=CtrStdNil?get_sdl_rect_ptr(argumentList->object):NULL;
  #ifdef NEW
  //TODO: Get color interface
  ctr_object* color_ = argumentList->next->object;
  uint8_t r,g,b;
  r = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  g = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  b = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  uint32_t color = SDL_MapRGB(surface->format, r, g, b);
  #else
  uint32_t color = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  #endif
  if(SDL_FillRect(surface, rect, color) == -1) {
    CtrStdFlow = ctr_build_string_from_cstring("Could not fill rect.");
    return CtrStdNil;
  }
  return myself;
}

ctr_object* ctr_sdl_surface_get_clip_rect(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = ctr_heap_allocate(sizeof(SDL_Rect));
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_GetClipRect(surface, rect);
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_RECT);
  instance->link = CtrStdSdl_rect;
  instance->value.rvalue->ptr = (void*)rect;
  return instance;
}


ctr_object* ctr_sdl_surface_scale(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  double amount = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  myself->value.rvalue->ptr = zoomSurface(surface, amount, amount, 1);
  SDL_FreeSurface(surface);
  return myself;
}
ctr_object* ctr_sdl_surface_scale_s(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  double w = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  double h = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  myself->value.rvalue->ptr = zoomSurface(surface, w/((double)surface->w), h/((double)surface->h), 1);
  SDL_FreeSurface(surface);
  return myself;
}


ctr_object* ctr_sdl_surface_set_color_key(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  uint32_t flag = SDL_SRCCOLORKEY | SDL_RLEACCEL;
  #ifdef NEW
  uint32_t color = 0;
  if(argumentList->next->object) {
    flag = ctr_internal_cast2bool(argumentList->next->object)->value.nvalue ? SDL_SRCCOLORKEY : 0;
  }
  if(flag != 0) {
    ctr_object* color_ = argumentList->object;
    uint8_t r,g,b;
    r = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
    g = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
    b = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
    color = SDL_MapRGB(surface->format, r, g, b);
  }
  #else
  uint32_t color = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  #endif
  if(SDL_SetColorKey(surface, flag, color) < 0) {
    char* err = strcat("setColorKey failed: ", SDL_GetError());
    CtrStdFlow = ctr_build_string_from_cstring(err);
  }
  return myself;
}


ctr_object* ctr_sdl_surface_update(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_Flip(on);
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
void ctr_sdl_get_pixel(SDL_Surface *surface, int x, int y, uint32_t* pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to get */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *pixel = *p;
        break;

    case 2:
        *pixel = *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            *pixel = p[0] << 16 | p[1] << 8 | p[2];
        } else {
          *pixel = p[0] | p[1] << 8 | p[2] << 16;
        }
        break;

    case 4:
        *pixel = *(Uint32 *)p;
        break;
    }
}
ctr_object* ctr_sdl_surface_set_pixel(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  int x, y;
  x = argumentList->object->value.nvalue;
  y = argumentList->next->object->value.nvalue;
  ctr_object* color_ = argumentList->next->next->object;
  uint8_t r,g,b;
  r = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  g = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  b = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  uint32_t color = SDL_MapRGB(surface->format, r, g, b);
  ctr_sdl_put_pixel(surface, x, y, color);
  return myself;
}
ctr_object* ctr_sdl_surface_get_pixel(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  int x, y;
  x = argumentList->object->value.nvalue;
  y = argumentList->next->object->value.nvalue;
  uint32_t pixel;
  ctr_sdl_get_pixel(surface, x, y, &pixel);
  uint8_t r,g,b;
  SDL_GetRGB(pixel, surface->format, &r, &g, &b);
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = CtrStdColor;
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("red"),   ctr_build_number_from_float(r), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("green"), ctr_build_number_from_float(g), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("blue"),  ctr_build_number_from_float(b), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("alpha"), ctr_build_number_from_float(0), CTR_CATEGORY_PRIVATE_PROPERTY);
  return instance;
}
#endif
/**
 * Event Interface
 *
 */
ctr_object* ctr_sdl_event_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_EVT);
  instance->link = myself;
  ((ctr_sdl_evt*)(instance->value.rvalue->ptr))->ptr = ctr_heap_allocate_tracked(sizeof(SDL_Event));
  return instance;
}

ctr_object* ctr_sdl_event_poll_event(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* cb = argumentList->object;
  ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
  if(SDL_PollEvent((SDL_Event*)(((ctr_sdl_evt*)(myself->value.rvalue->ptr))->ptr))) {
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
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_EVT);
  instance->link = CtrStdSdl_evt;
  //ctr_internal_create_func(instance, ctr_build_string_from_cstring("timestamp"), &ctr_sdl_event_key_get_timestamp);
  //ctr_internal_create_func(instance, ctr_build_string_from_cstring("windowID"), &ctr_sdl_event_key_get_windowID);
  // ctr_internal_create_func(instance, ctr_build_string_from_cstring("state"), &ctr_sdl_event_key_get_state);
  //ctr_internal_create_func(instance, ctr_build_string_from_cstring("repeat"), &ctr_sdl_event_key_get_repeat);
  // ctr_internal_create_func(instance, ctr_build_string_from_cstring("keysym"), &ctr_sdl_event_key_get_keysym);
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
  SDL_Event* evt = (SDL_Event*)(get_sdl_event_ptr(myself)->ptr);
  ctr_object* instance = ctr_build_number_from_float(evt->key.state);
  return instance;
}
// ctr_object* ctr_sdl_event_key_get_repeat (ctr_object* myself, ctr_argument* argumentList) {
//   ctr_object* instance = ctr_build_number_from_float(event.key.repeat);
//   return instance;
// }
ctr_object* ctr_sdl_event_key_get_keysym (ctr_object* myself, ctr_argument* argumentList) {
  SDL_Event* evt = (SDL_Event*)(get_sdl_event_ptr(myself)->ptr);
  ctr_object* instance = ctr_build_string_from_cstring(SDL_GetKeyName(evt->key.keysym.sym));
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
  SDL_Event* evt = (SDL_Event*)(get_sdl_event_ptr(myself)->ptr);
  return ctr_build_number_from_float(evt->type);
}

ctr_object* ctr_sdl_event_type_compare(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* other = ctr_internal_cast2number(argumentList->object);
  SDL_Event* evt = (SDL_Event*)(get_sdl_event_ptr(myself)->ptr);
  return ctr_build_bool(other!=NULL && (other->value.nvalue) == (double)(evt->type));
}


void begin() {
  atexit(ctr_sdl_quit_atexit);
  CtrStdSdl = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdSdl->link = CtrStdObject;
  CtrStdSdl_surface = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdSdl_surface->link = CtrStdObject;
  CtrStdSdl_rect = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdSdl_rect->link = CtrStdObject;
  CtrStdSdl_evt = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  CtrStdSdl_evt->link = CtrStdObject;
  CtrStdColor = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  //sdl, surface, rect
	ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "new" ), &ctr_sdl_make );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "init" ), &ctr_sdl_init );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "quit" ), &ctr_sdl_quit );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "set:to:" ), &ctr_sdl_set_prop);
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "loadImage:" ), &ctr_sdl_surface_loadImage );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "newRectWithX:andY:andW:andH:" ), &ctr_sdl_rect_make );

  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "blit:" ), &ctr_sdl_surface_blit );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "blit:fromRect:toRect:" ), &ctr_sdl_surface_blit_loc );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "update" ), &ctr_sdl_surface_update );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "setPixelWithX:andY:toColor:" ), &ctr_sdl_surface_set_pixel);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "pixelAtX:andY:" ), &ctr_sdl_surface_get_pixel);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "fillRect:withColor:" ), &ctr_sdl_surface_fill );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "clipRect" ), &ctr_sdl_surface_get_clip_rect );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "scale:" ), &ctr_sdl_surface_scale );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "scaleToW:andH:" ), &ctr_sdl_surface_scale_s );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "setColorKey:" ), &ctr_sdl_surface_set_color_key );

  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setX:"), &ctr_sdl_rect_alterx);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setY:"), &ctr_sdl_rect_altery);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setW:"), &ctr_sdl_rect_alterw);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setH:"), &ctr_sdl_rect_alterh);
  //event
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "new" ), &ctr_sdl_event_make );
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "poll:" ), &ctr_sdl_event_poll_event );
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "toNumber" ), &ctr_sdl_event_type);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "equals:" ), &ctr_sdl_event_type_compare);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "get:" ), &ctr_sdl_event_get_param);
  //
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring("keystate"), &ctr_sdl_event_key_get_state);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring("keysym"), &ctr_sdl_event_key_get_keysym);

  #ifdef NEW
  //Color
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "red:green:blue:alpha:" ), &ctr_sdl_color_make);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "rgba:" ), &ctr_sdl_color_make_rgba);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "rgb:" ), &ctr_sdl_color_make_rgb);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "setRed:green:blue:" ), &ctr_sdl_color_set_rgb);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "setRed:green:blue:alpha:" ), &ctr_sdl_color_set_rgb);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "get:" ), &ctr_sdl_color_get);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "set:to:" ), &ctr_sdl_color_set);
  ctr_internal_create_func(CtrStdColor, ctr_build_string_from_cstring( "toString" ), &ctr_sdl_color_to_string);

  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Color" ), CtrStdColor, 0);
  #endif
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDLEvent" ), CtrStdSdl_evt, 0);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDLSurface" ), CtrStdSdl_surface, 0);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDL" ), CtrStdSdl, 0);

  //types
  ctr_argument* ArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
	ArgumentList->next = NULL;
  ArgumentList->object = ctr_build_string_from_cstring( CTR_STD_EXTENSION_PATH "/extensions/sdl.ctr");
  ctr_object* ext = ctr_file_new(CtrStdFile, ArgumentList);
  ctr_file_include(ext, NULL);
  ctr_heap_free(ArgumentList);
}
