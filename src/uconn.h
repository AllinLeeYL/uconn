#ifndef _BOBLI_UCONN_H
#define _BOBLI_UCONN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <fcntl.h> //非阻塞socket
#include <unistd.h>
/*线程*/
//#include <thread>
//#include <mutex>

#include "utility.h"
#include "ubuff.h"
#include "foperator.h"

#define UCONN_INIT_SSTHRESH 32

#define UCONN_DEFAULT_WINDOWLEN 1 //默认窗口长度
#define UCONN_DEFAULT_GRAMLEN 2048 //默认报文长度
#define UCONN_BUFF_SIZE UCONN_DEFAULT_GRAMLEN*256*2+1 //默认缓冲区长度

#define UCONN_FSM_TIME_OUT 40 //状态机最大未出现状态转移的次数
#define UCONN_FSM_USLEEP_TIME 2000 //状态机睡眠时间

#define UCONN_NET_DELAY 32000 //报文往返延时
#define UCONN_RECV_MAX_TRY_TIME 400 //尝试接收的最大次数
#define UCONN_RECV_TRY_INTERVAL 5 //尝试接收的时间间隔

/*uheader_t控制位标记*/
#define UHEADER_CONTROL_FILENAME 0b10000000
#define UHEADER_CONTROL_FILEEOF  0b11000000

#define UHEADER_CONTROL_FILEDATA     0b01000000
#define UHEADER_CONTROL_URG      0b00100000
#define UHEADER_CONTROL_ACK      0b00010000
#define UHEADER_CONTROL_PSH      0b00001000
#define UHEADER_CONTROL_RST      0b00000100
#define UHEADER_CONTROL_SYN      0b00000010
#define UHEADER_CONTROL_FIN      0b00000001
#define UHEADER_CONTROL_ACK_SYN  0b00010010

//流量控制方法
enum TrafficControl{
    Stop_Wait,
    GBN
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
    SP_ACK_Wait_1
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
    int gramLen; //报文长度
    TrafficControl trafficControl; //流量控制
    TransmissionControl transmissionControl; //拥塞控制
    struct sockaddr remoteAddr; //远端连接地址

    Ubuff * ubuff;
    Ubuff * usendBuff;
    Useq * remoteSeq;
    pthread_t recvThreadID;
    //std::mutex threadMtx;
    //std::mutex mtx; //缓冲区锁
protected:
    int _uconnSetNonBlock();
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
    /*停等机制的函数*/
    int _uSendFile_1(FILE *, char *);
    int _uRecvFile_1(char *);
    /*滑动窗机制的函数*/
    int _uSendFile_2(FILE *, char *);
    int _uRecvFile_2(char *);
    /*简单连接的关闭方式*/
    int _uconnClose_1();
public:
    int windowLen; //窗口长度
    int ssthresh;
    Uconn(); //初始化
    Uconn(int); //初始化
    Uconn(TrafficControl, TransmissionControl);

    int ubindAddr(struct sockaddr *); //绑定本机地址

    int uconnAccept(); /*接受连接，超时没有连接则返回-1*/
    int uconnBuild(struct sockaddr *); /*建立连接，超时没有建立则返回-1*/

    //int uSendBuff(char *, int); /*发送缓冲区，若失败则返回-1*/
    //int uRecvBuff(char *, int); /*接受缓冲区，若失败则返回-1*/
    /*从远端接收UDP报文*/
    int _uconnRecvFrom(char *, int);
    /*向远端发送UDP报文*/
    int _uconnSendTo(char *, int);

    int uSendFile(FILE *, char *);
    int uRecvFile(char *);

    int isClosed();
    int isOpen();
    int uconnClose(); //关闭连接
    ~Uconn();
};

#endif