
#ifndef _BOBLI_UCONNECTION_C
#define _BOBLI_UCONNECTION_C 1
#include "uconnection.h"

uconn_t * uconnection(int __sockfd__, int __windowLen__, short __protocol__){
    uconn_t * __uc__;
    __uc__ = malloc(sizeof(uconn_t));
    __uc__->sockfd = __sockfd__;
    __uc__->windowLen = __windowLen__;
    __uc__->protocol = __protocol__;
    __uc__->_status = UCONN_STATUS_NOT_CONN;
    __uc__->_uconnAddr = malloc(sizeof(struct sockaddr));
    return __uc__;
}

void freeUconn(uconn_t * __uc__){
    free(__uc__);
}

int ubindSocket(uconn_t * __uc__, int __sockfd__){
    __uc__->sockfd = __sockfd__;
    //设置套接字为非阻塞
    if (set_nonblock(__uc__->sockfd) == -1){
        perror("set_nonblock error");
        return 0;
    }
    return 1;
}

int ubindSockaddr(uconn_t * __uc__, struct sockaddr * __uconnAddr__){
    if (bind(__uc__->sockfd, __uconnAddr__, sizeof(struct sockaddr)) < 0){
        perror("bind error");
        return 0;
    }
    return 1;
}

int closeUconn(uconn_t * __uc__){
    uheader_t uheader;
    uheader.SeqNum = __uc__->_localSeqNum;
    uheader.AckNum = __uc__->_remoteSeqNum;
    uheader.HeadLen = (uint8_t)sizeof(uheader_t);
    uheader.Control = UHEADER_CONTROL_FIN;
    uheader.Window = UCONN_DGRAM_SIZE;
    uheader.Emergency = 0;
    uheader.CheckSum = 0;
    uheader.CheckSum = ~_uComputeCheckSum((char *)&uheader, sizeof(uheader_t));
    sendto(__uc__->sockfd, &uheader, sizeof(uheader_t), 0, __uc__->_uconnAddr, sizeof(struct sockaddr));
    exit(0);
}

uint16_t _uComputeCheckSum(char * __udgram__, uconn_data_size __size__){
    uint32_t checkSum = 0;
    uconn_data_size i = 0;
    for (i = 0; i < __size__ - 1; i = i + 2){
        checkSum = *(uint16_t *)(__udgram__+i) + checkSum;
        while(checkSum / 0xffff != 0){
            checkSum = (checkSum / 0xffff) + (checkSum % 0xffff);
        }
    }
    if (i < __size__){
        char tempA[2];
        tempA[0] = __udgram__[i];
        tempA[1] = 0;
        checkSum = *(uint16_t *)(tempA) + checkSum;
        while(checkSum / 0xffff != 0){
            checkSum = (checkSum / 0xffff) + (checkSum % 0xffff);
        }
    }
    return (uint16_t)(checkSum % 0xffff);
}

#endif