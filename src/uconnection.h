#ifndef _BOBLI_UCONN_H
#define _BOBLI_UCONN_H 1

#include "Config.h"

#define UCONN_DGRAM_SIZE 2048

#define UCONN_PRO_STOPWAIT 0x0001
#define UCONN_PRO_FIXEDWINDOW 0x0010
#define UCONN_PRO_DYNAMICAL 0x0100

#define UCONN_STATUS_NOT_CONN 0x0001
#define UCONN_STATUS_CONNECTED 0x0010


#define UHEADER_CONTROL_URG 0b00100000
#define UHEADER_CONTROL_ACK 0b00010000
#define UHEADER_CONTROL_PSH 0b00001000
#define UHEADER_CONTROL_RST 0b00000100
#define UHEADER_CONTROL_SYN 0b00000010
#define UHEADER_CONTROL_FIN 0b00000001
#define UHEADER_CONTROL_ACK_SYN 0b00010010


#define uconn_data_size unsigned long   //以字节为单位

typedef struct{
    int sockfd;                     //绑定套接字
    int windowLen;                  //窗口长度
    uint16_t protocol;              //流量控制的方式
    uint16_t _status;               //当前连接状态
    struct sockaddr * _uconnAddr;   //远端连接地址
    uint32_t _localSeqNum;
    uint32_t _remoteSeqNum;
} uconn_t;

#pragma pack(1)
typedef struct {
    uint32_t SeqNum;
    uint32_t AckNum;
    uint8_t HeadLen;
    uint8_t Control;
    uint16_t Window;
    uint16_t CheckSum;
    uint16_t Emergency;
} uheader_t;
#pragma pack()

/*  初始化一个uconnect*/
uconn_t * uconnection(int __sockfd__, int __windowLen__, short __protocol__);
/*  释放uconnect*/
void freeUconn(uconn_t * __uc__);
/*  绑定Socket套接字
    并自动设置Socket套接字为非阻塞模式*/
int ubindSocket(uconn_t * __uc__, int __sockfd__);
/*  绑定本地地址
    通本机某一端口绑定，以监听udp数据报，绑定的本机地址由uconnAddr描述*/
int ubindSockaddr(uconn_t * __uc__, struct sockaddr * __uconnAddr__);
/*  建立连接
    通一个远端Socket建立连接，远端地址用uconnAddr描述*/
int buildUconn(uconn_t * __uc__, struct sockaddr * __uconnAddr__);
/*  接收连接
    监听本机接口，接受一个远端连接*/
int acceptUconn(uconn_t * __uc__);
/*  关闭一个uconnection连接*/
int closeUconn(uconn_t * __uc__);
/*  发送流数据
    发送一个字节流（也可以是二进制流）给远端
    适用于可以读入到内存的，比较小的文件发送*/
int uSendStream(uconn_t * __uc__, char * __stream__, uconn_data_size __size__);
/*  为要发送的数据块添加首部
    封装成uconnection协议的DGRAM*/
int _uPackUp(char * __buff__, long __dataLen__, uheader_t * __uheader__, char * __udgram__);
/*  计算校验和
    计算数据报首部+数据区的校验和*/
uint16_t _uComputeCheckSum(char * __udgram__, uconn_data_size __size__);

#endif