#include <stdlib.h>
#include <stdio.h>
#include <ffi.h>

void foo(float x) {
  printf("%f\n", x);
}

int main(int argc, char** argv) {
  ffi_cif* cif = malloc(sizeof(ffi_cif)); //<- Resource? yep
  void* arg_values[1]; //<- constructed on-demand
  ffi_type* arg_types[1]; //<- Resource
  *arg_types = &ffi_type_pointer;
  char* a = "test"; //Debate on malloc
  arg_values[0] = &a;
  if (ffi_prep_cif(cif, FFI_DEFAULT_ABI, 1, &ffi_type_sint, arg_types) != FFI_OK) {exit(1);} //FFI_?
  ffi_call(cif, FFI_FN(printf), NULL, arg_values); //Do shit with result...void should be NULL, handle the rest?
  free(cif);
}
