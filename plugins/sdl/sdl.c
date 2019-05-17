#ifdef existing
#include <Citron/citron.h>
#else
#warning "We don't have Citron installed"
#include "../../citron.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_rotozoom.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_gfxBlitFunc.h>

static int sdl_inited = 0;
static int ttf_inited = 0;
static int img_inited = 0;


#define CTR_SDL_TYPE_BASE 2

ctr_object* sdl_error(const char* errmsg, const char* cat) {//Copy errmsg, concat cat
  size_t errlen = strlen(errmsg);
  size_t catlen = strlen(cat);
  char* err = ctr_heap_allocate(sizeof(char)*(errlen + catlen + 1));
  memcpy(err, errmsg, errlen);
  memcpy(err+errlen, cat, catlen);
  ctr_object* errobj = ctr_build_string(err, errlen+catlen);
  ctr_heap_free(err);
  return errobj;
}

enum ctr_sdl {
  CTR_SDL_TYPE_SURFACE = CTR_SDL_TYPE_BASE,
  CTR_SDL_TYPE_RECT,
  CTR_SDL_TYPE_EVT,
  CTR_SDL_TYPE_FONT
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
ctr_object* CtrStdSdl_font;
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
 * SDL TTF Font interface
 */
ctr_object* ctr_sdl_ttf_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_ttf_open(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_ttf_render_solid(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_ttf_size(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_sdl_ttf_size_utf8(ctr_object* myself, ctr_argument* argumentList);
/*
 * SDL_gfx Primitives interface
 */

/**********************NATIVES***********************/
#define CTR_XYXYC_FUNCTION_BODY(name) SDL_Surface* dst = get_sdl_surface_ptr(myself);\
  ctr_object* color_obj;\
  Sint16 x1, x2, y1, y2;\
  Uint32 color = 0;\
  x1 = ctr_internal_cast2number(argumentList->object)->value.nvalue;\
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;\
  x2 = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;\
  y2 = ctr_internal_cast2number(argumentList->next->next->next->object)->value.nvalue;\
  color_obj = argumentList->next->next->next->next->object;\
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;\
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;\
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;\
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;\
  color |= ((Uint32)a);\
  color |= ((Uint32)b)<<8;\
  color |= ((Uint32)g)<<16;\
  color |= ((Uint32)r)<<24;\
  int errno = name (dst, x1, y1, x2, y2, color);\
  return myself;

/**/ // Pixel
/**/ // ctr_object* ctr_sdl_gfx_pixelColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Uint32 color);
/**/ // ctr_object* ctr_sdl_gfx_pixelRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Horizontal line
ctr_object* ctr_sdl_gfx_hlineColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
ctr_object* ctr_sdl_gfx_hlineRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Vertical line
ctr_object* ctr_sdl_gfx_vlineColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);
ctr_object* ctr_sdl_gfx_vlineRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Rectangle
ctr_object* ctr_sdl_gfx_rectangleColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
ctr_object* ctr_sdl_gfx_rectangleRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Filled rectangle Box
ctr_object* ctr_sdl_gfx_boxColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
ctr_object* ctr_sdl_gfx_boxRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Line
ctr_object* ctr_sdl_gfx_lineColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
ctr_object* ctr_sdl_gfx_lineRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// AA Line
ctr_object* ctr_sdl_gfx_aalineColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
ctr_object* ctr_sdl_gfx_aalineRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Circle
ctr_object* ctr_sdl_gfx_circleColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
ctr_object* ctr_sdl_gfx_circleRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// AA Circle
ctr_object* ctr_sdl_gfx_aacircleColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
ctr_object* ctr_sdl_gfx_aacircleRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Filled Circle
ctr_object* ctr_sdl_gfx_filledCircleColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
ctr_object* ctr_sdl_gfx_filledCircleRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Ellipse
ctr_object* ctr_sdl_gfx_ellipseColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
ctr_object* ctr_sdl_gfx_ellipseRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// AA Ellipse
ctr_object* ctr_sdl_gfx_aaellipseColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
ctr_object* ctr_sdl_gfx_aaellipseRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Filled Ellipse
ctr_object* ctr_sdl_gfx_filledEllipseColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
ctr_object* ctr_sdl_gfx_filledEllipseRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Pie
ctr_object* ctr_sdl_gfx_pieColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
ctr_object* ctr_sdl_gfx_pieRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Filled Pie
ctr_object* ctr_sdl_gfx_filledPieColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
ctr_object* ctr_sdl_gfx_filledPieRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Trigon
ctr_object* ctr_sdl_gfx_trigonColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
ctr_object* ctr_sdl_gfx_trigonRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// AA-Trigon
ctr_object* ctr_sdl_gfx_aatrigonColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
ctr_object* ctr_sdl_gfx_aatrigonRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst,  Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Filled Trigon
ctr_object* ctr_sdl_gfx_filledTrigonColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, int color);
ctr_object* ctr_sdl_gfx_filledTrigonRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Polygon
ctr_object* ctr_sdl_gfx_polygonColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint32 color);
ctr_object* ctr_sdl_gfx_polygonRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// AA-Polygon
ctr_object* ctr_sdl_gfx_aapolygonColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint32 color);
ctr_object* ctr_sdl_gfx_aapolygonRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Filled Polygon
ctr_object* ctr_sdl_gfx_filledPolygonColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, int color);
ctr_object* ctr_sdl_gfx_filledPolygonRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// Bezier Curve
ctr_object* ctr_sdl_gfx_bezierColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, int s, Uint32 color);
ctr_object* ctr_sdl_gfx_bezierRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, int s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// 8x8 Characters/Strings
ctr_object* ctr_sdl_gfx_characterColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint32 color);
ctr_object* ctr_sdl_gfx_characterRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
ctr_object* ctr_sdl_gfx_stringColor(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, char *c, Uint32 color);
ctr_object* ctr_sdl_gfx_stringRGBA(ctr_object* myself, ctr_argument* argumentList); //(SDL_Surface * dst, Sint16 x, Sint16 y, char *c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
// void gfxPrimitivesSetFont(unsigned char *fontdata, int cw, int ch);

/***********************Citron Interface****************************/

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
  if(img_inited) {
    IMG_Quit();
    img_inited = 0;
  }
  if(ttf_inited) {
    TTF_Quit();
    ttf_inited = 0;
  }
  if(sdl_inited) {
    SDL_Quit();
    sdl_inited = 0;
  }
}

/**
 *  SDL main entry point
 *
 *  SDL main functions
 */

ctr_object* ctr_sdl_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl);
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
  if(!sdl_inited) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
      (*get_CtrStdFlow()) = sdl_error("Unable to initialize SDL: ", SDL_GetError());
      ctr_heap_free(caption);
      return (*get_CtrStdFlow());
    }
    sdl_inited = 1;
  }
  SDL_WM_SetCaption (caption, caption);
  SDL_Surface* window = SDL_SetVideoMode(w, h, 0, 0);
  if (window == NULL) {
    (*get_CtrStdFlow()) = sdl_error("Unable to set Video Mode: ", SDL_GetError());
    ctr_heap_free(caption);
    return (*get_CtrStdFlow());
  }

  if(!img_inited) {
    // load support for the JPG and PNG image formats
    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if((initted&flags) != flags) {
      (*get_CtrStdFlow()) = sdl_error("IMG failed to initialize: ", IMG_GetError());
      return (*get_CtrStdFlow());
    }
    img_inited = 1;
  }
  if(!TTF_WasInit()) {
    int initted=TTF_Init();
    if(initted < 0) {
      (*get_CtrStdFlow()) = sdl_error("TTF failed to initialize: ", TTF_GetError());
      return (*get_CtrStdFlow());
    }
    ttf_inited = 1;
  }
  ctr_heap_free(caption);
  ctr_set_link_all(myself, CtrStdSdl_surface);
  myself->value.rvalue->ptr = (void*)window;
  return myself;
}
ctr_object* ctr_sdl_quit(ctr_object* myself, ctr_argument* argumentList) {
  if(img_inited) {
    IMG_Quit();
    img_inited = 0;
  }
  if(ttf_inited) {
    TTF_Quit();
    ttf_inited = 0;
  }
  if(sdl_inited) {
    SDL_Quit();
    sdl_inited = 0;
  }
  return CtrStdNil;
}

ctr_object* ctr_sdl_ticks(ctr_object* myself, ctr_argument* argumentList) {
  return ctr_build_number_from_float(SDL_GetTicks());
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
  ctr_set_link_all(instance, CtrStdSdl_rect);
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

ctr_object* ctr_sdl_rect_getx(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  return ctr_build_number_from_float(rect->x);
}
ctr_object* ctr_sdl_rect_gety(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  return ctr_build_number_from_float(rect->y);
}
ctr_object* ctr_sdl_rect_getw(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  return ctr_build_number_from_float(rect->w);
}
ctr_object* ctr_sdl_rect_geth(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  return ctr_build_number_from_float(rect->h);
}
ctr_object* ctr_sdl_rect_to_s(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = get_sdl_rect_ptr(myself);
  char str[1024];
  size_t len = sprintf(str, "Rect[%d,%d,%d,%d]", rect->x, rect->y, rect->w, rect->h);
  return ctr_build_string(str, len);
}
/**
 * SDL Color Interface
 *
 *
 * Color handlers
 */

 ctr_object* ctr_sdl_color_make(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
   ctr_set_link_all(instance, myself);
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
   (*get_CtrStdFlow()) = ctr_build_string_from_cstring("Expected an array of RGBA values");
   return myself;
   err_not_enough:
   (*get_CtrStdFlow()) = ctr_build_string_from_cstring("Expected an array of 4 values for RGBA");
   return myself;
 }
 ctr_object* ctr_sdl_color_set_rgb(ctr_object* myself, ctr_argument* argumentList) {
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("red"), (argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("green"), (argumentList->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("blue"), (argumentList->next->next->object), CTR_CATEGORY_PRIVATE_PROPERTY);
   ctr_internal_object_set_property(myself, ctr_build_string_from_cstring("alpha"), (argumentList->next->next->next->object?argumentList->next->next->next->object:ctr_build_number_from_float(255)), CTR_CATEGORY_PRIVATE_PROPERTY);
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
   (*get_CtrStdFlow()) = ctr_build_string_from_cstring("Expected an array of RGB values");
   return myself;
   err_not_enough:
   (*get_CtrStdFlow()) = ctr_build_string_from_cstring("Expected an array of 3 values for RGB");
   return myself;
 }

 ctr_object* ctr_sdl_color_set(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object *prop = ctr_internal_cast2string(argumentList->object), *val = ctr_internal_cast2number(argumentList->next->object);
   if (prop==NULL||val==NULL) {
     (*get_CtrStdFlow()) = prop==NULL?ctr_build_string_from_cstring("Propery name cannot be Nil."):ctr_build_string_from_cstring("Value must be a number.");
     return (*get_CtrStdFlow());
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

/**
 *  SDL surface entry point
 *
 *  SDL surface handler functions
 */

ctr_object* ctr_sdl_surface_loadImage(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  char* loc = ctr_heap_allocate_cstring (argumentList->object);
  if(access(loc, F_OK) == -1) {
    (*get_CtrStdFlow()) = ctr_build_string_from_cstring("specified file does not exist or cannot be accessed.");
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
  ctr_set_link_all(instance, CtrStdSdl_surface);
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
    (*get_CtrStdFlow()) = sdl_error("Blit failed: ", SDL_GetError());
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
  //TODO: Get color interface
  ctr_object* color_ = argumentList->next->object;
  uint8_t r,g,b;
  r = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("red"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  g = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("green"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  b = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("blue"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
  uint32_t color = SDL_MapRGB(surface->format, r, g, b);
  //uint32_t color = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  if(SDL_FillRect(surface, rect, color) != 0) {
    (*get_CtrStdFlow()) = sdl_error("Fill rect failed: ", SDL_GetError());
    return CtrStdNil;
  }
  return myself;
}

ctr_object* ctr_sdl_surface_get_clip_rect(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Rect* rect = ctr_heap_allocate(sizeof(SDL_Rect));
  SDL_Surface* surface = get_sdl_surface_ptr(myself);
  SDL_GetClipRect(surface, rect);
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_RECT);
  ctr_set_link_all(instance, CtrStdSdl_rect);
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
  // uint32_t color = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  if(SDL_SetColorKey(surface, flag, color) < 0) {
    (*get_CtrStdFlow()) = sdl_error("setColorKey failed: ", SDL_GetError());
  }
  return myself;
}


ctr_object* ctr_sdl_surface_update(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* on = get_sdl_surface_ptr(myself);
  SDL_Flip(on);
  SDL_UpdateRect(on, 0, 0, 0, 0);
  return myself;
}

ctr_object* ctr_sdl_surface_new(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surf = get_sdl_surface_ptr(myself);
  SDL_Surface* sf2 = SDL_CreateRGBSurface(0, surf->w, surf->h, surf->format->BitsPerPixel, surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
  if(!sf2){
    (*get_CtrStdFlow()) = sdl_error("CreateRGBSurface failed: ", SDL_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = sf2;
  return instance;
}
ctr_object* ctr_sdl_surface_new_wh(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surf = get_sdl_surface_ptr(myself);
  int w, h;
  w = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  h = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  SDL_Surface* sf2 = SDL_CreateRGBSurface(0, w, h, surf->format->BitsPerPixel, surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
  if(!sf2){
    (*get_CtrStdFlow()) = sdl_error("CreateRGBSurface failed: ", SDL_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = sf2;
  return instance;
}
#ifdef NEWSURF
ctr_object* ctr_sdl_surface_lock(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);

}
ctr_object* ctr_sdl_surface_unlock(ctr_object* myself, ctr_argument* argumentList) {
  SDL_Surface* surface = get_sdl_surface_ptr(myself);

}
#endif
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
  // a = ctr_internal_object_find_property(color_, ctr_build_string_from_cstring("alpha"), CTR_CATEGORY_PRIVATE_PROPERTY)->value.nvalue;
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
  ctr_set_link_all(instance, CtrStdColor);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("red"),   ctr_build_number_from_float(r), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("green"), ctr_build_number_from_float(g), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("blue"),  ctr_build_number_from_float(b), CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_set_property(instance, ctr_build_string_from_cstring("alpha"), ctr_build_number_from_float(0), CTR_CATEGORY_PRIVATE_PROPERTY);
  return instance;
}
/**
 * Event Interface
 *
 */
ctr_object* ctr_sdl_event_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_EVT);
  ctr_set_link_all(instance, myself);
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
ctr_object* ctr_sdl_event_get_ms(ctr_object* myself, ctr_argument* argumentList) {
  Uint32 state = SDL_GetMouseState(NULL, NULL);
  int
    left = state&SDL_BUTTON(SDL_BUTTON_LEFT),
    middle = state&SDL_BUTTON(SDL_BUTTON_MIDDLE),
    right = state&SDL_BUTTON(SDL_BUTTON_RIGHT);
  ctr_object* statemap = ctr_map_new(CtrStdMap, NULL);
  ctr_argument* argument = ctr_heap_allocate(sizeof(ctr_argument));
  argument->next = ctr_heap_allocate(sizeof(ctr_argument));
  argument->object = ctr_build_bool(left);
  argument->next->object = ctr_build_string_from_cstring("left");
  ctr_map_put(statemap, argument);
  argument->object = ctr_build_bool(middle);
  argument->next->object = ctr_build_string_from_cstring("middle");
  ctr_map_put(statemap, argument);
  argument->object = ctr_build_bool(right);
  argument->next->object = ctr_build_string_from_cstring("right");
  ctr_map_put(statemap, argument);
  ctr_heap_free(argument->next);
  ctr_heap_free(argument);
  return statemap;
}
ctr_object* ctr_sdl_event_get_mp(ctr_object* myself, ctr_argument* argumentList) {
  int x, y;
  SDL_GetMouseState(&x, &y);
  ctr_object* pos = ctr_array_new(CtrStdArray, NULL);
  argumentList->object = ctr_build_number_from_float(x);
  ctr_array_push(pos, argumentList);
  argumentList->object = ctr_build_number_from_float(y);
  ctr_array_push(pos, argumentList);
  return pos;
}
ctr_object* ctr_sdl_event_key_   (ctr_object* myself) {
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_EVT);
  ctr_set_link_all(instance, CtrStdSdl_evt);
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

/*
 * TTF Font Interface
 */
 ctr_object* ctr_sdl_ttf_make(ctr_object* myself, ctr_argument* argumentList) {
   ctr_object* container = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_FONT);
   ctr_set_link_all(container, CtrStdSdl_font);
   return container;
 }

 ctr_object* ctr_sdl_ttf_open(ctr_object* myself, ctr_argument* argumentList) { //Name, [size = 12]
   if(!TTF_WasInit()) {
     int initted=TTF_Init();
     if(initted < 0) {
       (*get_CtrStdFlow()) = sdl_error("TTF failed to initialize: ", TTF_GetError());
       return (*get_CtrStdFlow());
     }
   }
   char* fname = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
   int size = 12;
   if(argumentList->next->object)
     size = argumentList->next->object->value.nvalue;
   ctr_object* container = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_FONT);
   ctr_set_link_all(container, CtrStdSdl_font);
   TTF_Font* font = TTF_OpenFont(fname, size);
   ctr_heap_free(fname);
   if(!font) {
     (*get_CtrStdFlow()) = sdl_error("OpenFont error: ", TTF_GetError());
     return CtrStdNil;
   }
   container->value.rvalue->ptr = font;
   return container;
 }

ctr_object* ctr_sdl_ttf_render_solid(ctr_object* myself, ctr_argument* argumentList) {
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* ocolor = argumentList->next->object;
  SDL_Color color;
  color.r = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  color.g = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  color.b = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // color.a = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  char* text = ctr_heap_allocate_cstring(str);
  SDL_Surface* srf;
  if(str->value.svalue->vlen == strlen(text)) {//ASCII
    srf = TTF_RenderText_Solid(font, text, color);
  } else {//UTF8
    srf = TTF_RenderUTF8_Solid(font, text, color);
  }
  ctr_heap_free(text);
  if(!srf) {
    (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = srf;
  return instance;
}

ctr_object* ctr_sdl_ttf_renderu_solid(ctr_object* myself, ctr_argument* argumentList) {
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* ocolor = argumentList->next->object;
  SDL_Color color;
  color.r = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  color.g = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  color.b = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // color.a = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  char* text = ctr_heap_allocate_cstring(str);
  SDL_Surface*
    srf = TTF_RenderUTF8_Solid(font, text, color);
  ctr_heap_free(text);
  if(!srf) {
    (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = srf;
  return instance;
}
ctr_object* ctr_sdl_ttf_render_blended(ctr_object* myself, ctr_argument* argumentList) {
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* ocolor = argumentList->next->object;
  SDL_Color color;
  color.r = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  color.g = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  color.b = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // color.a = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  char* text = ctr_heap_allocate_cstring(str);
  SDL_Surface* srf;
  if(str->value.svalue->vlen == strlen(text)) {//ASCII
    srf = TTF_RenderText_Blended(font, text, color);
  } else {//UTF8
    srf = TTF_RenderUTF8_Blended(font, text, color);
  }
  ctr_heap_free(text);
  if(!srf) {
    (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = srf;
  return instance;
}

ctr_object* ctr_sdl_ttf_renderu_blended(ctr_object* myself, ctr_argument* argumentList) {
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* ocolor = argumentList->next->object;
  SDL_Color color;
  color.r = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  color.g = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  color.b = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // color.a = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  char* text = ctr_heap_allocate_cstring(str);
  SDL_Surface*
    srf = TTF_RenderUTF8_Blended(font, text, color);
  ctr_heap_free(text);
  if(!srf) {
    (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = srf;
  return instance;
}
ctr_object* ctr_sdl_ttf_render_shaded(ctr_object* myself, ctr_argument* argumentList) {
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* ocolor = argumentList->next->object;
  ctr_object* bocolor = argumentList->next->next->object;
  SDL_Color fcolor;
  fcolor.r = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  fcolor.g = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  fcolor.b = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // fcolor.a = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  SDL_Color bcolor;
  bcolor.r = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  bcolor.g = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  bcolor.b = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // bcolor.a = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  char* text = ctr_heap_allocate_cstring(str);
  SDL_Surface* srf;
  if(str->value.svalue->vlen == strlen(text)) {//ASCII
    srf = TTF_RenderText_Shaded(font, text, fcolor, bcolor);
  } else {//UTF8
    srf = TTF_RenderUTF8_Shaded(font, text, fcolor, bcolor);
  }
  ctr_heap_free(text);
  if(!srf) {
    (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = srf;
  return instance;
}

ctr_object* ctr_sdl_ttf_renderu_shaded(ctr_object* myself, ctr_argument* argumentList) {
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  ctr_object* ocolor = argumentList->next->object;
  ctr_object* bocolor = argumentList->next->next->object;
  SDL_Color fcolor;
  fcolor.r = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  fcolor.g = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  fcolor.b = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // fcolor.a = ctr_internal_object_find_property(ocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  SDL_Color bcolor;
  bcolor.r = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  bcolor.g = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  bcolor.b = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  // bcolor.a = ctr_internal_object_find_property(bocolor, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  char* text = ctr_heap_allocate_cstring(str);
  SDL_Surface*
  srf = TTF_RenderUTF8_Shaded(font, text, fcolor, bcolor);
  ctr_heap_free(text);
  if(!srf) {
    (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
    return CtrStdNil;
  }
  ctr_object* instance = ctr_sdl_create_container_of_type(CTR_SDL_TYPE_SURFACE);
  ctr_set_link_all(instance, CtrStdSdl_surface);
  instance->value.rvalue->ptr = srf;
  return instance;
}
ctr_object* ctr_sdl_ttf_size(ctr_object* myself, ctr_argument* argumentList) { // ([Font] text) -> [w, h]
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  char* text = ctr_heap_allocate_cstring(str);
  int w,h;
  int res = TTF_SizeText(font, text, &w, &h);
  ctr_heap_free(text);
  if (res) {
    // something is wrong
      (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
      return CtrStdNil;
  }
  ctr_object* tup = ctr_array_new(get_CtrStdArray(), NULL);
  ctr_array_push(tup, ctr_build_number_from_float(w));
  ctr_array_push(tup, ctr_build_number_from_float(h));
  tup->value.avalue->immutable = 1;
  return tup;
}
ctr_object* ctr_sdl_ttf_size_utf8(ctr_object* myself, ctr_argument* argumentList) { // ([Font] text) -> [w, h]
  TTF_Font* font = myself->value.rvalue->ptr;
  ctr_object* str = ctr_internal_cast2string(argumentList->object);
  char* text = ctr_heap_allocate_cstring(str);
  int w,h;
  int res = TTF_SizeUTF8(font, text, &w, &h);
  ctr_heap_free(text);
  if (res) {
    // something is wrong
      (*get_CtrStdFlow()) = sdl_error("Couldn't render text: ", TTF_GetError());
      return CtrStdNil;
  }
  ctr_object* tup = ctr_array_new(get_CtrStdArray(), NULL);
  ctr_array_push(tup, ctr_build_number_from_float(w));
  ctr_array_push(tup, ctr_build_number_from_float(h));
  tup->value.avalue->immutable = 1;
  return tup;
}
ctr_object* ctr_sdl_gfx_hlineColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x1, x2, y;
  Uint32 color = 0;
  x1 = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  x2 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  y = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = hlineColor(dst, x1, x2, y, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_vlineColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x, y1, y2;
  Uint32 color = 0;
  x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  y2 = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = vlineColor(dst, x, y1, y2, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_rectangleColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(rectangleColor)
}
ctr_object* ctr_sdl_gfx_boxColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(boxColor)
}
ctr_object* ctr_sdl_gfx_lineColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(lineColor)
}
ctr_object* ctr_sdl_gfx_aalineColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(aalineColor)
}
ctr_object* ctr_sdl_gfx_circleColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x, y1, r_;
  Uint32 color = 0;
  x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  r_ = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = circleColor(dst, x, y1, r_, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_aacircleColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x, y1, r_;
  Uint32 color = 0;
  x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  r_ = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = aacircleColor(dst, x, y1, r_, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_filledCircleColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x, y1, r_;
  Uint32 color = 0;
  x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  r_ = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = filledCircleColor(dst, x, y1, r_, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_ellipseColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(ellipseColor)
}
ctr_object* ctr_sdl_gfx_aaellipseColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(aaellipseColor)
}
ctr_object* ctr_sdl_gfx_filledEllipseColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
  CTR_XYXYC_FUNCTION_BODY(filledEllipseColor)
}
ctr_object* ctr_sdl_gfx_pieColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x, y1, rad, start, end;
  Uint32 color = 0;
  x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  rad = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  start = ctr_internal_cast2number(argumentList->next->next->next->object)->value.nvalue;
  end = ctr_internal_cast2number(argumentList->next->next->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = pieColor(dst, x, y1, rad, start, end, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_filledPieColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x, y1, rad, start, end;
  Uint32 color = 0;
  x = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  rad = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  start = ctr_internal_cast2number(argumentList->next->next->next->object)->value.nvalue;
  end = ctr_internal_cast2number(argumentList->next->next->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = filledPieColor(dst, x, y1, rad, start, end, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_trigonColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x1, y1, x2, y2, x3, y3;
  Uint32 color = 0;
  x1 = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  x2 = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  y2 = ctr_internal_cast2number(argumentList->next->next->next->object)->value.nvalue;
  x3 = ctr_internal_cast2number(argumentList->next->next->next->next->object)->value.nvalue;
  y3 = ctr_internal_cast2number(argumentList->next->next->next->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = trigonColor(dst, x1, y1, x2, y2, x3, y3, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_aatrigonColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x1, y1, x2, y2, x3, y3;
  Uint32 color = 0;
  x1 = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  x2 = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  y2 = ctr_internal_cast2number(argumentList->next->next->next->object)->value.nvalue;
  x3 = ctr_internal_cast2number(argumentList->next->next->next->next->object)->value.nvalue;
  y3 = ctr_internal_cast2number(argumentList->next->next->next->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = aatrigonColor(dst, x1, y1, x2, y2, x3, y3, color);
  return myself;
}
ctr_object* ctr_sdl_gfx_filledTrigonColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, int color);
  SDL_Surface* dst = get_sdl_surface_ptr(myself);
  ctr_object* color_obj;
  Sint16 x1, y1, x2, y2, x3, y3;
  Uint32 color = 0;
  x1 = ctr_internal_cast2number(argumentList->object)->value.nvalue;
  y1 = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
  x2 = ctr_internal_cast2number(argumentList->next->next->object)->value.nvalue;
  y2 = ctr_internal_cast2number(argumentList->next->next->next->object)->value.nvalue;
  x3 = ctr_internal_cast2number(argumentList->next->next->next->next->object)->value.nvalue;
  y3 = ctr_internal_cast2number(argumentList->next->next->next->next->next->object)->value.nvalue;
  color_obj = argumentList->next->next->next->next->next->next->object;
  int r = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("red"), 0)->value.nvalue;
  int g = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("green"), 0)->value.nvalue;
  int b = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("blue"), 0)->value.nvalue;
  int a = ctr_internal_object_find_property(color_obj, ctr_build_string_from_cstring("alpha"), 0)->value.nvalue;
  color |= ((Uint32)a);
  color |= ((Uint32)b)<<8;
  color |= ((Uint32)g)<<16;
  color |= ((Uint32)r)<<24;
  int errno = filledTrigonColor(dst, x1, y1, x2, y2, x3, y3, color);
  return myself;
}
// ctr_object* ctr_sdl_gfx_polygonColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint32 color);
//
// }
// ctr_object* ctr_sdl_gfx_aapolygonColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, Uint32 color);
//
// }
// ctr_object* ctr_sdl_gfx_filledPolygonColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, int color);
//
// }
// ctr_object* ctr_sdl_gfx_bezierColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 * vx, Sint16 * vy, int n, int s, Uint32 color);
//
// }
// ctr_object* ctr_sdl_gfx_characterColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, char c, Uint32 color);
//
// }
// ctr_object* ctr_sdl_gfx_stringColor(ctr_object* myself, ctr_argument* argumentList) { //(SDL_Surface * dst, Sint16 x, Sint16 y, char *c, Uint32 color);
//
// }
void begin() {
  atexit(ctr_sdl_quit_atexit);
  CtrStdSdl = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdSdl, CtrStdObject);
  CtrStdSdl_surface = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdSdl_surface, CtrStdObject);
  CtrStdSdl_rect = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdSdl_rect, CtrStdObject);
  CtrStdSdl_evt = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdSdl_evt, CtrStdObject);
  CtrStdColor = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdColor, CtrStdObject);
  CtrStdSdl_font = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  ctr_set_link_all(CtrStdSdl_font, CtrStdObject);
  //sdl, surface, rect
	ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "new" ), &ctr_sdl_make );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "init" ), &ctr_sdl_init );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "quit" ), &ctr_sdl_quit );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "ticks" ), &ctr_sdl_ticks );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "set:to:" ), &ctr_sdl_set_prop);
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "loadImage:" ), &ctr_sdl_surface_loadImage );
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring( "newRectWithX:andY:andW:andH:" ), &ctr_sdl_rect_make );

  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "free" ), &ctr_sdl_surface_free_surface );
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
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "newBuffer" ), &ctr_sdl_surface_new );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "newBufferWithWidth:andHeight:" ), &ctr_sdl_surface_new_wh );
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "hline_x0:x1:y0:color:" ), &ctr_sdl_gfx_hlineColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "vline_x0:x1:y0:color:" ), &ctr_sdl_gfx_vlineColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "rectangle_x0:y0:x1:y1:color:" ), &ctr_sdl_gfx_rectangleColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "box_x0:y0:x1:y1:color:" ), &ctr_sdl_gfx_boxColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "line_x0:y0:x1:y1:color:" ), &ctr_sdl_gfx_lineColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "aaline_x0:y0:x1:y1:color:" ), &ctr_sdl_gfx_aalineColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "circle_x0:y0:r0:color:" ), &ctr_sdl_gfx_circleColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "aacircle_x0:y0:r0:color:" ), &ctr_sdl_gfx_aacircleColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "filledCircle_x0:y0:r0:color:" ), &ctr_sdl_gfx_filledCircleColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "ellipse_x0:y0:r0:r1:color:" ), &ctr_sdl_gfx_ellipseColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "aaellipse_x0:y0:r0:r1:color:" ), &ctr_sdl_gfx_aaellipseColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "filledEllipse_x0:y0:r0:r1:color:" ), &ctr_sdl_gfx_filledEllipseColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "pie_x0:y0:rad:start:end:color:" ), &ctr_sdl_gfx_pieColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "filledPie_x0:y0:rad:start:end:color:" ), &ctr_sdl_gfx_filledPieColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "trigon_x0:y0:x1:y1:x2:y2:color:" ), &ctr_sdl_gfx_trigonColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "aatrigon_x0:y0:x1:y1:x2:y2:color:" ), &ctr_sdl_gfx_aatrigonColor);
  ctr_internal_create_func(CtrStdSdl_surface, ctr_build_string_from_cstring( "filledTrigon_x0:y0:x1:y1:x2:y2:color:" ), &ctr_sdl_gfx_filledTrigonColor);

  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setX:"), &ctr_sdl_rect_alterx);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setY:"), &ctr_sdl_rect_altery);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setW:"), &ctr_sdl_rect_alterw);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "setH:"), &ctr_sdl_rect_alterh);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "getX"), &ctr_sdl_rect_getx);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "getY"), &ctr_sdl_rect_gety);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "getW"), &ctr_sdl_rect_getw);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "getH"), &ctr_sdl_rect_geth);
  ctr_internal_create_func(CtrStdSdl_rect, ctr_build_string_from_cstring( "toString"), &ctr_sdl_rect_to_s);
  //event
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "new" ), &ctr_sdl_event_make );
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "poll:" ), &ctr_sdl_event_poll_event );
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "toNumber" ), &ctr_sdl_event_type);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "equals:" ), &ctr_sdl_event_type_compare);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "get:" ), &ctr_sdl_event_get_param);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "mouseState" ), &ctr_sdl_event_get_ms);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring( "mousePosition" ), &ctr_sdl_event_get_mp);
  //
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring("keystate"), &ctr_sdl_event_key_get_state);
  ctr_internal_create_func(CtrStdSdl_evt, ctr_build_string_from_cstring("keysym"), &ctr_sdl_event_key_get_keysym);
  //font
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring("loadFont:"), &ctr_sdl_ttf_open);
  ctr_internal_create_func(CtrStdSdl, ctr_build_string_from_cstring("loadFont:size:"), &ctr_sdl_ttf_open);
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("renderSolid:color:"), &ctr_sdl_ttf_render_solid);
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("renderSolidUnicode:color:"), &ctr_sdl_ttf_renderu_solid);
  //--
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("renderBlended:color:"), &ctr_sdl_ttf_render_blended);
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("renderBlendedUnicode:color:"), &ctr_sdl_ttf_renderu_blended);
  //--
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("renderShaded:fore:back:"), &ctr_sdl_ttf_render_shaded);
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("renderShadedUnicode:fore:back:"), &ctr_sdl_ttf_renderu_shaded);
  //--
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("textSize:"), &ctr_sdl_ttf_size);
  ctr_internal_create_func(CtrStdSdl_font, ctr_build_string_from_cstring("textSizeUnicode:"), &ctr_sdl_ttf_size_utf8);
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
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDLEvent" ), CtrStdSdl_evt, 0);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDLSurface" ), CtrStdSdl_surface, 0);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SDL" ), CtrStdSdl, 0);

  //types
  ctr_argument* ArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
	ArgumentList->next = NULL;
  char s[1024];
  int len = sprintf(s, "%s/extensions/sdl.ctr", ctr_file_stdext_path_raw());
  ArgumentList->object = ctr_build_string(s, len);
  ctr_object* ext = ctr_file_new(CtrStdFile, ArgumentList);
  ctr_file_include(ext, NULL);
  ctr_heap_free(ArgumentList);
}
