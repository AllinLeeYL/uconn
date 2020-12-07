#include "utility.h"

void bstrcpy(char * __des__, char * __src__, uint32_t __length__){
    for (uint32_t i = 0; i < __length__; i++){
        __des__[i] = __src__[i];
    }
}