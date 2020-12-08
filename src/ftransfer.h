#ifndef _BOBLI_FTRANSFER_H
#define _BOBLI_FTRANSFER_H

#include "uconn.h"
#include "foperator.h"
#include <netinet/in.h>

#define FTRANSFER_BLOCK_SIZE 1024
#define FTRANSFER_TRY_TIMES 5

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
    FILE * fp;
    fspliter_t * fsp;
    char filename[FTRANSFER_BLOCK_SIZE];
    Uconn * uconn;
    struct sockaddr remoteAddr;
    int timer;
public:
    ftransfer();
    ~ftransfer();
    void bindAddr(struct sockaddr *);
    void bindRemoteAddr(struct sockaddr *);
    int open(char *, char *); //打开的文件名，打开的方式
    int close();
    int Send(char *, struct sockaddr *); //发送文件名，远端地址
    int Recv(char *, char *);//第一个参数指定存储目录，第二个用于存储返回的文件名
};

#endif