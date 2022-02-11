#ifndef WITH_AUTOMAKE
#    ifndef CTR_STD_EXTENSION_PATH
#        define CTR_STD_EXTENSION_PATH "."
#    endif
#else
#    ifndef CTR_STD_EXTENSION_PATH
#        define CTR_STD_EXTENSION_PATH WITH_AUTOMAKE
#    endif
#endif
