#include "citron.h"

/**
 * import <name>
 *
 * Library for searching paths for objects and dynamically loading them
 */
ctr_object *
ctr_importlib_begin (ctr_object * myself, ctr_argument * argumentList)
{
  ctr_object *instance = ctr_internal_create_object (CTR_OBJECT_TYPE_OTOBJECT);
  instance->link = myself;
  CtrStdImportLib_SearchPaths = ctr_array_new (CtrStdArray, NULL);
  ctr_invoke_variadic (CtrStdImportLib_SearchPaths, &ctr_array_push, 1,
		       ctr_build_string_from_cstring ("."));
  ctr_invoke_variadic (CtrStdImportLib_SearchPaths, &ctr_array_push, 1,
		       ctr_build_string_from_cstring (CTR_STD_EXTENSION_PATH));
  ctr_internal_object_add_property (instance,
				    ctr_build_string_from_cstring
				    ("searchPaths"),
				    CtrStdImportLib_SearchPaths, CTR_CATEGORY_PRIVATE_PROPERTY);
  ctr_internal_object_add_property (CtrStdWorld,
				    ctr_build_string_from_cstring ("import"), instance, 0);
  ctr_file_include_here (ctr_invoke_variadic (CtrStdFile, &ctr_file_new, 1, ctr_build_string_from_cstring (CTR_STD_EXTENSION_PATH "/extensions/importlib.ctr")), NULL);	//get all definitions
  return instance;
}
