#include "utility.h"

void bstrcpy(char * __des__, char * __src__, int __length__){
    for (int i = 0; i < __length__; i++){
        __des__[i] = __src__[i];
    }
}