#include "utility.h"

void bstrcpy(char * __des__, char * __src__, int __length__){
    for (int i = 0; i < __length__; i++){
        __des__[i] = __src__[i];
    }
}

void sleepnsec(unsigned long nsec){
    struct timespec slptime;
    slptime.tv_sec = nsec / 1000000000;
    slptime.tv_nsec = nsec % 1000000000;
    nanosleep(&slptime, NULL);
}