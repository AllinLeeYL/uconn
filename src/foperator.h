#ifndef _BOBLI_FOPERATOR_H
#define _BOBLI_FOPERATOR_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utility.h"

typedef struct{
    FILE * fp;                      
    uint32_t fileSize;                  //文件大小
    uint32_t blockSize;                 //块大小
    uint32_t curptr;                    //当前位置(字节位置)
} fspliter_t;

/*  切分文件
    用fspliter切分一个文件，返回值的文件大小，
    若返回0，则文件为空或出错*/
fspliter_t * fsplit(FILE * __fp__, uint32_t __blockSize__);
/*  获取切分的下一块
    将__fsplt__的下一块内容写入buff，并返回块长度(字节大小)
    请确保buff由足够的长度(__blockSize__)以写入
    返回0表示到文件尾，返回值小于0出错*/
long fsplt_next(char * __buff__, fspliter_t * __fsplt__, long __size__);

#endif