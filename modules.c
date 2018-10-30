#define MODULE_TCL 1


#if MODULE_TCL
  #define begin ctr_mod_tcl_begin
  #include "plugins/tcl/tcl.c"
  #undef begin
#endif



void
ctr_preload_modules()
{
#if MODULE_TCL
  ctr_mod_tcl_begin();
#endif
}
