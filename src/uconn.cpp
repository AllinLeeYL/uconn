#include "uconn.h"

int Uconn::_uconnSetNonBlock(){
    int opts=fcntl(this->sockfd, F_GETFL);
    if(opts < 0){
        perror("fcntl(sock,GETFL)");
        return -1;
    }

    opts = opts|O_NONBLOCK;
    if(fcntl(sockfd, F_SETFL, opts) < 0){
 		perror("fcntl(sock,SETFL,opts)");
        return -1;
    }
    return 1;
}

int Uconn::_uconnRecvFrom(void * _buff_, int _N_){
    socklen_t socklen = sizeof(struct sockaddr);
    int n = recvfrom(this->sockfd, _buff_, _N_, 0, &(this->remoteAddr), &socklen);
    return n;
}

int Uconn::_uconnSendTo(void * _buff_, int _N_){
    socklen_t socklen = sizeof(struct sockaddr);
    int n = sendto(this->sockfd, _buff_, _N_, 0, &(this->remoteAddr), socklen);
    return n;
}

int Uconn::_uconnCheckHeader(uheader_t * _uheader_){
    if (_uheader_->HeadLen != 16){
        return -1;
    }
    return (int)(_uheader_->DataLen);
}

int Uconn::_uconnCheckGram(char * _gram_, int _len_){
    uheader_t * uheader;
    uheader  = (uheader_t *)_gram_;
    uint16_t checkSum = uheader->CheckSum;
    uheader->CheckSum = 0;
    uint16_t computeCheckSum = this->_uconnComputeCheckSum(_gram_, _len_);
    uheader->CheckSum = checkSum;
    if (checkSum != computeCheckSum){
        return -1;
    }
    return uheader->DataLen;
}

uint16_t Uconn::_uconnComputeCheckSum(char * _gram_, int _size_){
    uint32_t checkSum = 0;
    int i = 0;
    for (i = 0; i < _size_ - 1; i = i + 2){
        checkSum = *(uint16_t *)(_gram_+i) + checkSum;
        while(checkSum / 0xffff != 0){
            checkSum = (checkSum / 0xffff) + (checkSum % 0xffff);
        }
    }
    if (i < _size_){
        char tempA[2];
        tempA[0] = _gram_[i];
        tempA[1] = 0;
        checkSum = *(uint16_t *)(tempA) + checkSum;
        while(checkSum / 0xffff != 0){
            checkSum = (checkSum / 0xffff) + (checkSum % 0xffff);
        }
    }
    return (uint16_t)(checkSum % 0xffff);
}

int Uconn::_uconnAccept_1(){
    this->sendTimer = 0;
    char * buff; //接收缓冲区
    uheader_t * uheader; //数据报头部，指向接收缓冲区
    buff = (char *)malloc(this->ubuff->buffSize);
    memset(buff, 0, this->ubuff->buffSize);
    uheader = (uheader_t *)buff;

    while (this->sendTimer < UCONN_TIME_OUT){
        switch (this->commonState){
            case Closed:{
                this->commonState = Listen;
                this->sendTimer = 0;
                break;
            }
            case Listen:{
                uheader = (uheader_t *)buff;
                int n = this->_uconnRecvFrom(buff, sizeof(uheader_t));
                if (n < sizeof(uheader_t)){
                    break;
                }
                n = this->_uconnCheckHeader((uheader_t *)buff);
                if (n != 0){
                    break;
                }
                if (this->_uconnCheckGram(buff, sizeof(uheader_t)) < 0){
                    break;
                }
                if (uheader->Control != UHEADER_CONTROL_SYN){
                    break;
                }
                this->commonState = SYN_Recv_1;
                this->remoteSeq->seq = uheader->SeqNum;
                this->sendTimer = 0;
                break;
            }
            case SYN_Recv_1:{
                uheader_t suheader;

                this->ubuff->setInitPtr(rand()%(this->ubuff->buffSize));
                suheader.SeqNum = this->ubuff->endptr;
                suheader.AckNum = this->remoteSeq->seq;
                suheader.HeadLen = 16;
                suheader.Control = UHEADER_CONTROL_ACK_SYN;
                suheader.Window = 0;
                suheader.CheckSum = 0;
                suheader.DataLen = 0;
                suheader.CheckSum = this->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));

                this->_uconnSendTo(&suheader, sizeof(uheader_t));
                this->commonState = Established;
                this->sendTimer = 0;
                break;
            }
            case Established:{
                //创建接收者线程
                if (this->_uRecvBuff() < 0){
                    perror("RecvBuff error");
                    break;
                }
                this->recvState = Recv_Established;
                this->sendState = Send_Established;
                return 1;
                break;
            }
            default:{
                break;
            }
        }
        this->sendTimer = this->sendTimer + 1;
        usleep(UCONN_USLEEP_TIME);
    }
    free(buff);
    return -1;
}

int Uconn::_uconnBuild_1(struct sockaddr * _addr_){
    bstrcpy((char *)&(this->remoteAddr), (char *)_addr_, sizeof(struct sockaddr));
    this->sendTimer = 0;
    char * buff; //接收缓冲区
    uheader_t * uheader; //数据报头部，指向接收缓冲区
    buff = (char *)malloc(this->ubuff->buffSize);
    memset(buff, 0, this->ubuff->buffSize);
    uheader = (uheader_t *)buff;
    this->commonState = Closed;

    while (this->sendTimer < UCONN_TIME_OUT){
        switch (this->commonState){
            case Closed:{
                this->commonState = SYN_Sent;
                this->sendTimer = 0;
                break;
            }
            case SYN_Sent:{
                uheader_t suheader;

                this->ubuff->setInitPtr(rand()%(this->ubuff->buffSize));
                suheader.SeqNum = this->ubuff->endptr;
                suheader.AckNum = 0;
                suheader.HeadLen = 16;
                suheader.Control = UHEADER_CONTROL_SYN;
                suheader.Window = 0;
                suheader.CheckSum = 0;
                suheader.DataLen = 0;
                suheader.CheckSum = this->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));
                
                this->_uconnSendTo(&suheader, sizeof(uheader_t));

                uheader = (uheader_t *)buff;
                int n = this->_uconnRecvFrom(buff, sizeof(uheader_t));
                if (n < sizeof(uheader_t)){
                    break;
                }
                n = this->_uconnCheckHeader((uheader_t *)buff);
                if (n != 0){
                    break;
                }
                if (this->_uconnCheckGram(buff, sizeof(uheader_t)) < 0){
                    break;
                }
                if (uheader->Control != UHEADER_CONTROL_ACK_SYN){
                    break;
                }
                this->commonState = Established;
                this->sendTimer = 0;
                break;
            }
            case Established:{
                //创建接收者线程
                if (this->_uRecvBuff() < 0){
                    perror("RecvBuff error");
                    break;
                }
                this->recvState = Recv_Established;
                this->sendState = Send_Established;
                return 1;
                break;
            }
            default:{
                break;
            }
        }
        this->sendTimer = this->sendTimer + 1;
        usleep(UCONN_USLEEP_TIME);
    }

    free(buff);
    return -1;
}

int Uconn::_uSendBuff_1(char * _buff_, int _len_){
    this->sendTimer = 0;

    while (this->sendTimer < UCONN_TIME_OUT){
        switch (this->sendState){
            default:
                break;
        }
        
        this->sendTimer = this->sendTimer + 1;
        usleep(UCONN_USLEEP_TIME);
    }
}
//创建接收者线程
int Uconn::_uRecvBuff(){
    if (this->trafficControl == Stop_Wait){
        if(pthread_create(&(this->recvThreadID), NULL , Uconn::_recvThread_1, this) == -1){
            perror("Thread Error");
            return -1;
        }
    }
    return 0;
}

int Uconn::_uconnClose_1(){

}
/*接收者线程，负责接收来自发送端的报文，维护一个接收缓冲区*/
void *Uconn::_recvThread_1(void * _this_){
    Uconn * uconn_p;
    int n, m;
    uconn_p = (Uconn *)_this_;
    uconn_p->recvTimer = 0;
    char * buff; //接收缓冲区
    uheader_t * uheader; //数据报头部，指向接收缓冲区
    buff = (char *)malloc(uconn_p->ubuff->buffSize);
    memset(buff, 0, uconn_p->ubuff->buffSize);
    uheader = (uheader_t *)buff;

    while (uconn_p->recvTimer < UCONN_TIME_OUT){
        switch (uconn_p->recvState){
            case Recv_Established:{
                //先读取头部
                n = uconn_p->_uconnRecvFrom(buff, sizeof(uheader_t));
                if (n < sizeof(uheader_t)){
                    break;
                }
                //检查头部
                n = uconn_p->_uconnCheckHeader((uheader_t *)buff);
                if (n <= 0){
                    break;
                }
                //再读取数据
                m = uconn_p->_uconnRecvFrom((void *)(buff + sizeof(uheader_t)), n);
                if (m != n){
                    break;
                }
                //检查校验和
                if (uconn_p->_uconnCheckGram(buff, sizeof(uheader_t) + n) < 0){
                    break;
                }
                //正确的报文
                if (uheader->AckNum == uconn_p->ubuff->endptr){
                    uconn_p->remoteSeq->seq = uheader->SeqNum;
                    uconn_p->recvState = SP_RecvBUFF_Check;
                    uconn_p->recvTimer = 0;
                    break;
                }//旧的报文
                else if (*(uconn_p->remoteSeq) > uheader->AckNum){
                    uheader_t suheader;
                    suheader.SeqNum = uconn_p->ubuff->endptr;
                    suheader.AckNum = uconn_p->remoteSeq->seq;
                    suheader.HeadLen = 16;
                    suheader.Control = UHEADER_CONTROL_ACK;
                    suheader.Window = 0;
                    suheader.CheckSum = 0;
                    suheader.DataLen = 0;
                    suheader.CheckSum = uconn_p->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));
                    uconn_p->_uconnSendTo(&suheader, sizeof(uheader_t));
                    uconn_p->recvTimer = 0;
                }
                //不正确的报文直接忽略
                break;
            }
            case SP_RecvBUFF_Check:{
                uheader_t suheader;
                if (uconn_p->mtx.try_lock() == false){
                    break;
                }
                //缓冲区不足
                if (uconn_p->ubuff->remainSize() < (n + sizeof(uheader_t))){
                    uconn_p->mtx.unlock();
                    break;
                }
                //缓冲区充足
                uconn_p->ubuff->write(buff, n + sizeof(uheader_t));
                uconn_p->mtx.unlock();

                suheader.SeqNum = uconn_p->ubuff->endptr;
                suheader.AckNum = uconn_p->remoteSeq->seq;
                suheader.HeadLen = 16;
                suheader.Control = UHEADER_CONTROL_ACK;
                suheader.Window = 0;
                suheader.CheckSum = 0;
                suheader.DataLen = 0;
                suheader.CheckSum = uconn_p->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));
                uconn_p->_uconnSendTo(&suheader, sizeof(uheader_t));
                break;
            }
            default:{
                break;
            }
        }
        uconn_p->recvTimer = uconn_p->recvTimer + 1;
        usleep(UCONN_USLEEP_TIME);
    }

    printf("recvThread End\n");
}

Uconn::Uconn(){
    this->commonState = Closed;
    this->sendState = Send_Established;
    this->recvState = Recv_Established;
    this->trafficControl = Stop_Wait;
    this->transmissionControl = default_control;
    memset((void *)(&(this->remoteAddr)), 0, sizeof(struct sockaddr));
    this->sendTimer = 0;
    this->recvTimer = 0;
    this->sockfd = socket(AF_INET , SOCK_DGRAM , 0);
    this->windowLen = UCONN_DEFAULT_WINDOWLEN;
    this->gramLen = UCONN_DEFAULT_GRAMLEN;
    this->remoteSeq = new Useq(UCONN_BUFF_SIZE);
    this->ubuff = new Ubuff(UCONN_BUFF_SIZE);
    this->recvThreadID = 0;

    this->_uconnSetNonBlock();
}

int Uconn::ubindAddr(struct sockaddr * _uconnAddr_){
    if (bind(this->sockfd, _uconnAddr_, sizeof(struct sockaddr)) < 0){
        perror("bind error");
        return 0;
    }
    return 1;
}
/*接受连接，超时没有连接则返回-1*/
int Uconn::uconnAccept(){
    return this->_uconnAccept_1();
}
/*建立连接，超时没有建立则返回-1*/
int Uconn::uconnBuild(struct sockaddr * _addr_){
    return this->_uconnBuild_1(_addr_);
}
/*发送缓冲区，若失败则返回-1*/
int Uconn::uSendBuff(char * _buff_, int _len_){
    if (this->trafficControl == Stop_Wait){
        return this->_uSendBuff_1(_buff_, _len_);
    }
}

int Uconn::uconnClose(){
    return this->_uconnClose_1();
}

Uconn::~Uconn(){
    pthread_join(this->recvThreadID, NULL);
    delete this->ubuff;
    delete this->remoteSeq;
}
