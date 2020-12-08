#include "foperator.h"

fspliter_t * fsplit(FILE * __fp__, uint32_t __blockSize__){
    if (__fp__ == NULL){
        return 0;
    }
    fspliter_t * __fsplt__;
    __fsplt__ = (fspliter_t *)malloc(sizeof(fspliter_t));
    //初始化fspliter
    __fsplt__->fp = __fp__;
    fseek(__fp__, 0, SEEK_END);
    __fsplt__->fileSize = ftell(__fp__);
    fseek(__fp__, 0, SEEK_SET);
    __fsplt__->blockSize = __blockSize__;
    __fsplt__->curptr = 0;

    return __fsplt__;
}

long fsplt_next(char * __buff__, fspliter_t * __fsplt__, long __size__){
    long leftLength = __fsplt__->fileSize - __fsplt__->curptr;
    long readSize = 0;
    if (__buff__ == NULL || __fsplt__ == NULL){
        return -1;
    }
    if (leftLength == 0){
        return 0;
    }
    readSize = __size__ > 0 ? __size__ : __fsplt__->blockSize;
    if (leftLength < __size__){
        fread(__buff__, leftLength, 1, __fsplt__->fp);
        __fsplt__->curptr = __fsplt__->fileSize;
        return leftLength;
    }
    else{
        fread(__buff__, __size__, 1, __fsplt__->fp);
        __fsplt__->curptr = __fsplt__->curptr + __size__;
        return __size__;
    }
}