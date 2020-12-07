#ifndef _BOBLI_UCONN_H
#define _BOBLI_UCONN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <fcntl.h> //非阻塞socket
#include <unistd.h>
/*线程*/
#include <thread>
#include <mutex>

#include "utility.h"
#include "ubuff.h"

#define UCONN_DEFAULT_WINDOWLEN 1024 //默认窗口长度
#define UCONN_DEFAULT_GRAMLEN 1024 //默认报文长度
#define UCONN_BUFF_SIZE 65536 //默认缓冲区长度
#define UCONN_TIME_OUT 600 //2秒超时
#define UCONN_USLEEP_TIME 10000 //10毫秒刷新

/*uheader_t控制位标记*/
#define UHEADER_CONTROL_URG 0b00100000
#define UHEADER_CONTROL_ACK 0b00010000
#define UHEADER_CONTROL_PSH 0b00001000
#define UHEADER_CONTROL_RST 0b00000100
#define UHEADER_CONTROL_SYN 0b00000010
#define UHEADER_CONTROL_FIN 0b00000001
#define UHEADER_CONTROL_ACK_SYN 0b00010010

//流量控制方法
enum TrafficControl{
    Stop_Wait
};

//拥塞控制方法
enum TransmissionControl{
    default_control
};

//uconn公有状态
enum CommonState{
    Closed,
    //建立连接状态
    Listen,
    SYN_Sent,
    SYN_Recv_1,
    SYN_Recv_2,
    Established,
    //关闭连接状态
    Close_Wait,
    Closing,
    ACK_Wait,
    FIN_Wait
};

//uconn发送状态
enum SendState{
    Send_Established,
    //停等机制状态
    SP_BUFF_Check,
    SP_ACK_Wait_1,
    SP_ACK_Wait_2
};

//uconn接收状态
enum RecvState{
    Recv_Established,
    //停等机制状态
    SP_RecvBUFF_Check
};

/*uconn报头格式*/
#pragma pack(1)
typedef struct {
    uint32_t SeqNum; //同步序列号（本机期望收到的下一个序列号）
    uint32_t AckNum; //确认序列号（远端缓冲区的序列号）
    uint8_t HeadLen; //首部长度（字节为单位）
    uint8_t Control; //标志位，SYN,ACK等
    uint16_t Window; //窗口长度
    uint16_t CheckSum; //校验和
    uint16_t DataLen; //数据区长度（字节为单位），最大数据长度65535Bytes
} uheader_t;
#pragma pack()

//uconn连接，基于UDP的可靠传输实现
class Uconn{
protected:
    /*状态机*/
    CommonState commonState;
    SendState sendState;
    RecvState recvState;
    
    int sendTimer;
    int recvTimer;
    int sockfd; //套接字
    int windowLen; //窗口长度
    int gramLen; //报文长度
    TrafficControl trafficControl; //流量控制
    TransmissionControl transmissionControl; //拥塞控制
    struct sockaddr remoteAddr; //远端连接地址

    Ubuff * ubuff;
    Useq * remoteSeq;
    pthread_t recvThreadID;
    std::mutex mtx;
protected:

    int _uconnSetNonBlock();
    /*从远端接收UDP报文*/
    int _uconnRecvFrom(void *, int);
    /*向远端发送UDP报文*/
    int _uconnSendTo(void *, int);
    /*检查报头，包括报头长度
    若报文不正确，则返回-1，否则返回报文长度，
    报文长度可能是0.*/
    int _uconnCheckHeader(uheader_t *);
    /*检查整个数据报
    主要针对校验和进行计算，若报文错误，返回-1，否则返回报文长度*/
    int _uconnCheckGram(char *, int);
    /*计算校验和
    必须保证原数据报校验和位为0*/
    uint16_t _uconnComputeCheckSum(char *, int);

    /*简单双向连接的建立方式*/
    int _uconnAccept_1();
    int _uconnBuild_1(struct sockaddr *);
    /*停等机制的发送者函数*/
    int _uSendBuff_1(char *, int);

    int _uRecvBuff(); //接受缓冲区传输
    /*简单连接的关闭方式*/
    int _uconnClose_1();
public:
    /*停等机制的接收者线程*/
    static void *_recvThread_1(void *);
public:
    Uconn(); //初始化
    Uconn(TrafficControl, TransmissionControl);

    int ubindAddr(struct sockaddr *); //绑定本机地址

    int uconnAccept(); /*接受连接，超时没有连接则返回-1*/
    int uconnBuild(struct sockaddr *); /*建立连接，超时没有建立则返回-1*/

    int uSendBuff(char *, int); /*发送缓冲区，若失败则返回-1*/

    int uconnClose(); //关闭连接
    ~Uconn();
};

#endif