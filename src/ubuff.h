#ifndef _BOBLI_UBUFF_H
#define _BOBLI_UBUFF_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

class Useq{
public:
    uint32_t seq;
    uint32_t size;
public:
    Useq();
    ~Useq();
    Useq(uint32_t);
    uint32_t operator+(const Useq &) const;
    uint32_t operator+(uint32_t) const;
    int operator=(const Useq &);
    int operator=(uint32_t);

    /*这是一个循环序列号，可以看成一个圆圈。
    若序列号相等，则相等。
    相加距离更近，则小于；相减距离更近，则大于。
    若加减距离相等，则大于。*/
    int operator<(const Useq &) const;
    int operator<(uint32_t) const;
    int operator>(const Useq &) const;
    int operator>(uint32_t) const;
};

class Ubuff{
public:
    uint32_t curptr;
    uint32_t endptr;
    uint32_t buffSize;
    char * buff;
public:
    Ubuff();
    Ubuff(uint32_t);
    int setInitPtr(uint32_t);
    int size(); //返回缓冲区已存储的字节大小
    int remainSize(); //返回剩余缓冲区大小
    int write(char *, uint32_t); //写入字节为n的缓冲区
    int read(char *, uint32_t); //读取字节为n的缓冲区，不删除
    int get(char *, uint32_t); //获取字节为n的缓冲区，删除

    int operator<(uint32_t) const;
    int operator<=(uint32_t) const;
    int operator>(uint32_t) const;
    ~Ubuff();
};

#endif