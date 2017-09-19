#include "../../citron.h"
#include <stdio.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#define SFML_DYNAMIC

/** [SFML] new
 *
 *  Simple, Fast Multimedia Library
 *
 *  Creates an object representing an SFML window's prototype
 *
 */
 ctr_object* ctr_sfml_make (ctr_object* myself, ctr_argument* argumentList) {
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
		ctr_build_string_from_cstring( "windowColorDepth" ),
		ctr_build_number_from_float(32),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	ctr_internal_object_set_property(
		instance,
		ctr_build_string_from_cstring( "windowName" ),
		ctr_build_string_from_cstring( "CTR:SFML Window" ),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
  return instance;
 }

 /** [SFML] setWindowProperty: [String] to: [Object]
  *
  */
  ctr_object* ctr_sfml_set_prop (ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* pname = argumentList->object;
    if (pname->info.type != CTR_OBJECT_TYPE_OTSTRING || argumentList->object->value.svalue->vlen == 0) {
      CtrStdFlow = ctr_build_string_from_cstring ( "setWindowProperty key needs to be a non-empty string." );
      return CtrStdFlow;
    }
    ctr_object* pset  = argumentList->next->object;
    ctr_internal_object_set_property(
  		myself,
  		pname,
      pset,
  		CTR_CATEGORY_PRIVATE_PROPERTY
  	);
    return myself;
  }

/** [SFML] init
 *
 */
 ctr_object* ctr_sfml_initialize (ctr_object* myself, ctr_argument* argumentList) {
   int h = ctr_internal_object_find_property(
 		myself,
 		ctr_build_string_from_cstring( "windowHeight" ),
 		CTR_CATEGORY_PRIVATE_PROPERTY
  )->value.nvalue;
   int w = ctr_internal_object_find_property(
     myself,
     ctr_build_string_from_cstring( "windowWidth" ),
     CTR_CATEGORY_PRIVATE_PROPERTY
    )->value.nvalue;
  int depth = ctr_internal_object_find_property(
    myself,
    ctr_build_string_from_cstring( "windowColorDepth" ),
    CTR_CATEGORY_PRIVATE_PROPERTY
    )->value.nvalue;
   ctr_object* ctrname = ctr_internal_object_find_property(
     myself,
     ctr_build_string_from_cstring( "windowName" ),
     CTR_CATEGORY_PRIVATE_PROPERTY
     );

   sfVideoMode mode = {w, h, depth};
   sfRenderWindow* window = sfRenderWindow_create (mode, ctrname->value.svalue->value, sfResize|sfClose, NULL);
   if (!window) {
     CtrStdFlow = ctr_build_string_from_cstring ( "Could not initialize window." );
     return CtrStdFlow;
   }
   ctr_internal_object_set_property (
     myself,
     ctr_build_string_from_cstring ( "windowPtr" ),
     (ctr_object*) window,
     CTR_CATEGORY_PRIVATE_PROPERTY
   );
   return myself;
 }

 /**
  * C-constructor function.
  *
  */
 void begin(){
  printf("Loaded sfml.");
 	ctr_object* sfmlObject = ctr_sfml_make(CtrStdObject, NULL);
 	sfmlObject->link = CtrStdObject;

 	ctr_internal_create_func(sfmlObject, ctr_build_string_from_cstring( "new" ), &ctr_sfml_make );
 	ctr_internal_create_func(sfmlObject, ctr_build_string_from_cstring( "init" ), &ctr_sfml_initialize );
 	ctr_internal_create_func(sfmlObject, ctr_build_string_from_cstring( "setWindowProperty:to:" ), &ctr_sfml_set_prop );
 	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "SFML" ), sfmlObject, 0);
 }
