#ifndef _BOBLI_FTRANSFER_H
#define _BOBLI_FTRANSFER_H

#include "uconn.h"

#define FTRANSFER_FILENAME_LEN 1024

#define FTRHEADER_PROTOCOL 0x19 //协议标识位
#define FTRHEADER_CONTROL_FILENAME 0x00 //文件名
#define FTRHEADER_CONTROL_FILEDATA 0x01 //文件数据
#define FTRHEADER_CONTROL_FILEEOF 0x10 //文件结束

/*ftransfer包头格式*/
#pragma pack(1)
typedef struct {
    uint8_t protocol; //声明使用的协议
    uint8_t control; //控制位
    uint16_t length; //数据区长度
} ftrheader_t;
#pragma pack()

class ftransfer{
protected:
    FILE * fp; //文件
    char filename[FTRANSFER_FILENAME_LEN];
    long int fileSize; //文件大小
    long int cur; //当前位置偏移量
public:
    ftransfer();
    int ftropen(char *, char *); //打开的文件名，打开的方式
    int ftrSend(char *, struct sockaddr *); //发送文件名，远端地址
    int ftrRecv(char *, char *);//第一个参数指定存储目录，第二个用于存储返回的文件名
};

#endif