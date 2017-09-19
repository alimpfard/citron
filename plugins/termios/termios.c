#include <unistd.h>
#include <stdio.h>
#include "../../citron.h"
#include <termios.h>

struct termios options;

ctr_object* ctr_termios_make(ctr_object* myself, ctr_argument* argumentList) {
  ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
  return instance;
}

ctr_object* ctr_termios_nc(ctr_object* myself, ctr_argument* argumentList) {
  options.c_lflag |= ICANON;
  return CtrStdNil;
}
ctr_object* ctr_termios_c(ctr_object* myself, ctr_argument* argumentList) {
  options.c_lflag &= ~ICANON;
  return CtrStdNil;
}

void begin() {
  if(tcgetattr(stdin, &options) < 0) {
        //perror("stdin");
        return;
  }
  ctr_object* __termios = ctr_termios_make(CtrStdObject, NULL);
  ctr_internal_create_func(__termios, ctr_build_string_from_cstring("setNonCanonical"), &ctr_termios_nc);
  ctr_internal_create_func(__termios, ctr_build_string_from_cstring("setCanonical"), &ctr_termios_c);
  ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring("Termios"), __termios, 0);
}
