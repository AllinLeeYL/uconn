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

int Uconn::_uconnRecvFrom(char * _buff_, int _N_){
    socklen_t socklen = sizeof(struct sockaddr);
    int n = recvfrom(this->sockfd, _buff_, _N_, 0, &(this->remoteAddr), &socklen);
    return n;
}

int Uconn::_uconnSendTo(char * _buff_, int _N_){
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

    while (this->sendTimer < UCONN_FSM_TIME_OUT){
        switch (this->commonState){
            case Closed:{
                this->commonState = Listen;
                this->sendTimer = 0;
                break;
            }
            case Listen:{
                uheader = (uheader_t *)buff;
                int n = this->_uconnRecvFrom(buff, this->gramLen);
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
                this->usendBuff->setInitPtr(uheader->SeqNum);//记录远端序列号
                this->sendTimer = 0;
                break;
            }
            case SYN_Recv_1:{
                uheader_t suheader;
                srand((unsigned)time(NULL));
                this->ubuff->setInitPtr(rand()%(this->ubuff->buffSize));
                suheader.SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
                suheader.AckNum = this->usendBuff->curptr; //远端序列号
                suheader.HeadLen = 16;
                suheader.Control = UHEADER_CONTROL_ACK_SYN;
                suheader.Window = this->windowLen;
                suheader.CheckSum = 0;
                suheader.DataLen = 0;
                suheader.CheckSum = this->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));

                this->_uconnSendTo((char *)(&suheader), this->gramLen); //发送两次以增大连接建立几率
                this->_uconnSendTo((char *)(&suheader), this->gramLen);
                this->commonState = Established;
                this->sendTimer = 0;
                break;
            }
            case Established:{
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
        usleep(UCONN_FSM_USLEEP_TIME);
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

    while (this->sendTimer < UCONN_FSM_TIME_OUT){
        switch (this->commonState){
            case Closed:{
                this->commonState = SYN_Sent;
                this->sendTimer = 0;
                break;
            }
            case SYN_Sent:{
                uheader_t suheader;
                srand((unsigned)time(NULL));
                this->ubuff->setInitPtr(rand()%(this->ubuff->buffSize));
                suheader.SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
                suheader.AckNum = 0; //远端序列号
                suheader.HeadLen = 16;
                suheader.Control = UHEADER_CONTROL_SYN;
                suheader.Window = this->windowLen;
                suheader.CheckSum = 0;
                suheader.DataLen = 0;
                suheader.CheckSum = this->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));
                
                this->_uconnSendTo((char *)(&suheader), this->gramLen);

                usleep(UCONN_NET_DELAY);
                uheader = (uheader_t *)buff;
                for (int try_time = 0; try_time < UCONN_RECV_MAX_TRY_TIME; try_time = try_time + 1){
                    int n = this->_uconnRecvFrom(buff, this->gramLen);
                    if (n < (int)sizeof(uheader_t)){
                        break;
                    }
                    n = this->_uconnCheckHeader((uheader_t *)buff);
                    if (n != 0){
                        break;
                    }
                    if (this->_uconnCheckGram(buff, sizeof(uheader_t)) < 0){
                        break;
                    }
                    usleep(UCONN_RECV_TRY_INTERVAL);
                }
                if (uheader->Control != UHEADER_CONTROL_ACK_SYN){
                    break;
                }
                this->usendBuff->setInitPtr(uheader->SeqNum); //记录远端序列号
                this->commonState = Established;
                this->sendTimer = 0;
                break;
            }
            case Established:{
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
        usleep(UCONN_FSM_USLEEP_TIME);
    }

    free(buff);
    return -1;
}

int Uconn::_uconnClose_1(){
    uheader_t suheader;
    suheader.SeqNum = this->ubuff->endptr;
    suheader.AckNum = this->usendBuff->curptr;
    suheader.HeadLen = 16;
    suheader.Control = UHEADER_CONTROL_FIN;
    suheader.Window = this->windowLen;
    suheader.CheckSum = 0;
    suheader.DataLen = 0;
    suheader.CheckSum = this->_uconnComputeCheckSum((char *)(&suheader), sizeof(uheader_t));
    this->_uconnSendTo((char *)&suheader, gramLen);
    return 1;
}

Uconn::Uconn(){
    this->commonState = Closed;
    this->sendState = Send_Established;
    this->recvState = Recv_Established;
    this->trafficControl = GBN;
    this->transmissionControl = default_control;
    memset((void *)(&(this->remoteAddr)), 0, sizeof(struct sockaddr));
    this->sendTimer = 0;
    this->recvTimer = 0;
    this->sockfd = socket(AF_INET , SOCK_DGRAM , 0);
    this->windowLen = UCONN_DEFAULT_WINDOWLEN;
    this->gramLen = UCONN_DEFAULT_GRAMLEN;
    this->remoteSeq = new Useq(UCONN_BUFF_SIZE);
    this->ubuff = new Ubuff(UCONN_BUFF_SIZE);
    this->usendBuff = new Ubuff(UCONN_BUFF_SIZE);
    this->recvThreadID = 0;

    this->_uconnSetNonBlock();
}

Uconn::Uconn(int _n_){
    this->commonState = Closed;
    this->sendState = Send_Established;
    this->recvState = Recv_Established;
    this->trafficControl = _n_ == 0 ? Stop_Wait : GBN;
    this->transmissionControl = default_control;
    memset((void *)(&(this->remoteAddr)), 0, sizeof(struct sockaddr));
    this->sendTimer = 0;
    this->recvTimer = 0;
    this->sockfd = socket(AF_INET , SOCK_DGRAM , 0);
    this->windowLen = UCONN_DEFAULT_WINDOWLEN;
    this->gramLen = UCONN_DEFAULT_GRAMLEN;
    this->remoteSeq = new Useq(UCONN_BUFF_SIZE);
    this->ubuff = new Ubuff(UCONN_BUFF_SIZE);
    this->usendBuff = new Ubuff(UCONN_BUFF_SIZE);
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
int Uconn::uSendFile(FILE * fp, char * _filename_){
    this->sendState = SP_BUFF_Check;
    if (this->trafficControl == Stop_Wait){
        return this->_uSendFile_1(fp, _filename_);
    }
    else if (this->trafficControl == GBN){
        return this->_uSendFile_2(fp, _filename_);
    }
    return -1;
}

int Uconn::uRecvFile(char * _filename_){
    if (this->trafficControl == Stop_Wait){
        return this->_uRecvFile_1(_filename_);
    }
    else if (this->trafficControl == GBN){
        return this->_uRecvFile_2(_filename_);
    }
    return -1;
}
//发送文件，停等协议
int Uconn::_uSendFile_1(FILE * fp, char * _filename_){
    this->windowLen = 1;
    return this->_uSendFile_2(fp, _filename_);
}
//发送文件，滑动窗口协议
int Uconn::_uSendFile_2(FILE * fp, char * _filename_){
    uint32_t blockSize = this->gramLen - sizeof(uheader_t);
    int readSize = 0;
    uint32_t sendPercentage = 0;
    fspliter_t * fsp = fsplit(fp, blockSize);
    char * readbuff, * windowbuff, * sendbuff, * recvbuff;
    uheader_t * suheader, * ruheader;
    readbuff = (char *)malloc(blockSize);
    memset(readbuff, 0, blockSize);
    windowbuff = (char *)malloc(blockSize * this->windowLen);
    memset(windowbuff, 0, blockSize * this->windowLen);
    sendbuff = (char *)malloc(this->gramLen);
    memset(sendbuff, 0, this->gramLen);
    recvbuff = (char *)malloc(this->gramLen);
    memset(recvbuff, 0, this->gramLen);
    suheader = (uheader_t *)sendbuff;
    ruheader = (uheader_t *)recvbuff;

    //文件名
    suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
    suheader->AckNum = this->usendBuff->curptr; //远端序列号
    suheader->HeadLen = 16;
    suheader->Control = UHEADER_CONTROL_FILENAME;
    suheader->Window = this->windowLen;
    suheader->CheckSum = 0;
    suheader->DataLen = uint16_t(strlen(_filename_) + 1); //文件名长度包括'\0'
    strcpy((char *)(sendbuff + sizeof(uheader_t)), _filename_); //将文件名复制到发送缓冲区
    suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
    for (this->sendTimer = 0;this->sendTimer < UCONN_FSM_TIME_OUT; this->sendTimer = this->sendTimer + 1){
        this->_uconnSendTo(sendbuff, this->gramLen);
        usleep(UCONN_NET_DELAY);
        for (int n = 0; n < UCONN_RECV_MAX_TRY_TIME; n = n + 1){
            usleep(UCONN_RECV_TRY_INTERVAL);
            int len = this->_uconnRecvFrom(recvbuff, this->gramLen);
            if (len <= 0 || this->_uconnCheckGram(recvbuff, this->gramLen) < 0){
                //数据报校验：判断是否是数据报
                continue;
            }
            if (ruheader->AckNum != this->ubuff->endptr || ruheader->Control != UHEADER_CONTROL_ACK){
                //正确性校验：判断报文是否正确
                continue;
            }
            //收到文件名的ACK
            this->usendBuff->setInitPtr(ruheader->SeqNum);
            goto _uSendFile_2_FILEDATA;
        }
    }
    if (this->sendTimer >= UCONN_FSM_TIME_OUT){
        return -1;
    }

    //文件数据
    _uSendFile_2_FILEDATA:
    printf("文件名发送成功\n");
    sendPercentage = 0;
    readSize = fsplt_next(readbuff, fsp, blockSize);
    while (readSize > 0 || this->usendBuff->size() > 0){
        if (fsp->curptr * 10/ fsp->fileSize >= sendPercentage){
            //发送进度显示
            printf("FINISH: %d0%\n", sendPercentage);
            sendPercentage = sendPercentage + 2;
        }
        if (readSize == blockSize){
            //文件仍未读取完毕
            if (this->usendBuff->size() < this->windowLen * blockSize){
                //窗口未填满
                this->usendBuff->write(readbuff, blockSize);
                readSize = fsplt_next(readbuff, fsp, blockSize);
                continue;
            }
            else{
                //窗口已经填满
                this->usendBuff->read(windowbuff, blockSize * this->windowLen);
                for (uint32_t i = 0; i < (uint32_t)(blockSize * this->windowLen); i = i + blockSize){
                    *(this->remoteSeq) = this->usendBuff->curptr;
                    suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
                    suheader->AckNum = *(this->remoteSeq) + i; //远端序列号
                    suheader->HeadLen = 16;
                    suheader->Control = UHEADER_CONTROL_FILEDATA;
                    suheader->Window = this->windowLen;
                    suheader->CheckSum = 0;
                    suheader->DataLen = blockSize; //文件名长度包括'\0'
                    bstrcpy((char *)(sendbuff + sizeof(uheader_t)), (char *)(windowbuff + i), blockSize);
                    suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
                    this->_uconnSendTo(sendbuff, this->gramLen);
                }
                //接收ACK
                usleep(UCONN_NET_DELAY);
                for (int n = 0; n < 2 * this->windowLen; n = n + 1){
                    usleep(UCONN_RECV_TRY_INTERVAL);
                    int len = this->_uconnRecvFrom(recvbuff, this->gramLen);
                    if (len <= 0 || this->_uconnCheckGram(recvbuff, this->gramLen) < 0){
                        //数据报校验：判断是否是数据报
                        continue;
                    }
                    if (ruheader->AckNum != this->ubuff->endptr || ruheader->Control != UHEADER_CONTROL_ACK){
                        //正确性校验：判断报文是否正确
                        continue;
                    }
                    //报文正确
                    this->usendBuff->curptr = ruheader->SeqNum;
                }
                continue;
            }
        }
        else{
            //文件读取完毕，只剩最后一块或完全读完，只需发送剩余缓冲区
            if (this->usendBuff->size() < this->windowLen * blockSize && readSize > 0){
                //缓冲区未填满，读取最后一块数据
                this->usendBuff->write(readbuff, readSize);
                readSize = fsplt_next(readbuff, fsp, blockSize);
                continue;
            }
            else{
                //缓冲区已满或者最后一块数据已经读取完毕
                this->usendBuff->read(windowbuff, this->usendBuff->size());
                for (uint32_t i = 0; i < this->usendBuff->size(); i = i + blockSize){
                    int dataLen = this->usendBuff->size() - i < blockSize ? this->usendBuff->size() - i : blockSize;
                    *(this->remoteSeq) = this->usendBuff->curptr;
                    suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
                    suheader->AckNum = *(this->remoteSeq) + i; //远端序列号
                    suheader->HeadLen = 16;
                    suheader->Control = UHEADER_CONTROL_FILEDATA;
                    suheader->Window = this->windowLen;
                    suheader->CheckSum = 0;
                    suheader->DataLen = dataLen;
                    bstrcpy((char *)(sendbuff + sizeof(uheader_t)), (char *)(windowbuff + i), dataLen);
                    suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
                    this->_uconnSendTo(sendbuff, this->gramLen);
                }
                //接收ACK
                usleep(UCONN_NET_DELAY);
                for (int n = 0; n < 2 * this->windowLen; n = n + 1){
                    usleep(UCONN_RECV_TRY_INTERVAL);
                    int len = this->_uconnRecvFrom(recvbuff, this->gramLen);
                    if (len <= 0 || this->_uconnCheckGram(recvbuff, this->gramLen) < 0){
                        //数据报校验：判断是否是数据报
                        continue;
                    }
                    if (ruheader->AckNum != this->ubuff->endptr || ruheader->Control != UHEADER_CONTROL_ACK){
                        //正确性校验：判断报文是否正确
                        continue;
                    }
                    //报文正确
                    this->usendBuff->curptr = ruheader->SeqNum;
                }
                continue;
            }
        }
    }
    //文件结尾
    printf("文件数据发送成功！\n");
    suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
    suheader->AckNum = this->usendBuff->curptr; //远端序列号
    suheader->HeadLen = 16;
    suheader->Control = UHEADER_CONTROL_FILEEOF;
    suheader->Window = this->windowLen;
    suheader->CheckSum = 0;
    suheader->DataLen = uint16_t(0); //文件名长度包括'\0'
    suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
    for (this->sendTimer = 0;this->sendTimer < UCONN_FSM_TIME_OUT; this->sendTimer = this->sendTimer + 1){
        this->_uconnSendTo(sendbuff, this->gramLen);
        usleep(UCONN_NET_DELAY);
        for (int n = 0; n < UCONN_RECV_MAX_TRY_TIME; n = n + 1){
            usleep(UCONN_RECV_TRY_INTERVAL);
            int len = this->_uconnRecvFrom(recvbuff, this->gramLen);
            if (len <= 0 || this->_uconnCheckGram(recvbuff, this->gramLen) < 0){
                //数据报校验：判断是否是数据报
                continue;
            }
            if (ruheader->AckNum != this->ubuff->endptr || ruheader->Control != UHEADER_CONTROL_ACK){
                //正确性校验：判断报文是否正确
                continue;
            }
            //收到文件结尾的ACK
            this->usendBuff->setInitPtr(ruheader->SeqNum);
            goto _uSendFile_2_FILEEOF;
        }
    }
    if (this->sendTimer >= UCONN_FSM_TIME_OUT){
        return -1;
    }

    _uSendFile_2_FILEEOF:
    printf("文件发送成功！\n");
    free(readbuff);
    free(sendbuff);
    free(recvbuff);
}
//接收文件，停等协议
int Uconn::_uRecvFile_1(char * _filename_){
    this->windowLen = 1;
    return this->_uRecvFile_2(_filename_);
}

//接收文件，滑动窗口协议
int Uconn::_uRecvFile_2(char * _filename_){
    uint32_t blockSize = this->gramLen - sizeof(uheader_t);
    char * sendbuff, * recvbuff, * blockbuff;
    FILE * fp = NULL;
    int readSize = 0;
    uheader_t * suheader, * ruheader;
    sendbuff = (char *)malloc(this->gramLen);
    memset(sendbuff, 0, this->gramLen);
    recvbuff = (char *)malloc(this->gramLen);
    memset(recvbuff, 0, this->gramLen);
    blockbuff = (char *)malloc(blockSize);
    memset(blockbuff, 0, blockSize);
    suheader = (uheader_t *)sendbuff;
    ruheader = (uheader_t *)recvbuff;

    //文件名
    for (this->recvTimer = 0;this->recvTimer < UCONN_FSM_TIME_OUT*10; this->recvTimer = this->recvTimer + 1){
        usleep(UCONN_FSM_USLEEP_TIME);
        int len = this->_uconnRecvFrom(recvbuff, this->gramLen);
        if (len <= 0 || this->_uconnCheckGram(recvbuff, this->gramLen) < 0){
            //数据报校验：判断是否是数据报
            continue;
        }
        if (ruheader->AckNum != this->ubuff->endptr || ruheader->Control != UHEADER_CONTROL_FILENAME){
            //正确性校验：判断报文是否正确
            continue;
        }
        goto _uRecvFile_2_FILEDATA;
    }
    if (this->recvTimer >= UCONN_FSM_TIME_OUT*10){
        return -1;
    }

    /*文件数据
    先根据文件名打开文件，再传送数据*/
    _uRecvFile_2_FILEDATA:
    //回复ACK并同步远端序列号
    this->usendBuff->setInitPtr(ruheader->SeqNum); //设置远端序列号
    suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
    suheader->AckNum = this->usendBuff->curptr; //远端序列号
    suheader->HeadLen = 16;
    suheader->Control = UHEADER_CONTROL_ACK;
    suheader->Window = this->windowLen;
    suheader->CheckSum = 0;
    suheader->DataLen = uint16_t(0); //文件名长度包括'\0'
    suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
    this->_uconnSendTo(sendbuff, this->gramLen); //发送两次以避免丢失
    this->_uconnSendTo(sendbuff, this->gramLen);
    usleep(UCONN_NET_DELAY);
    //打开文件
    printf("收到文件名\n");
    strcpy(blockbuff, "copy_");
    strcat(blockbuff, (char *)(recvbuff + sizeof(uheader_t)));
    strcpy(_filename_, blockbuff);
    if (fp != NULL){
        fclose(fp);
    }
    fp = fopen(blockbuff, "wb");
    //开始传送数据
    for (this->recvTimer = 0; this->recvTimer < UCONN_FSM_TIME_OUT*10; this->recvTimer = this->recvTimer + 1){
        for (int n = 0; n < UCONN_RECV_MAX_TRY_TIME; n = n + 1){
            usleep(UCONN_RECV_TRY_INTERVAL);
            int len = this->_uconnRecvFrom(recvbuff, this->gramLen);
            if (len <= 0 || this->_uconnCheckGram(recvbuff, this->gramLen) < 0){
                //数据报校验：判断是否是数据报
                continue;
            }
            if (*(this->ubuff) > ruheader->AckNum){
                //正确性校验：判断报文是否正确
                continue;
            }
            if (ruheader->Control == UHEADER_CONTROL_FILENAME){
                //接收到的仍是文件名，回送ACK
                goto _uRecvFile_2_FILEDATA;
            }
            if (ruheader->Control == UHEADER_CONTROL_FILEEOF){
                goto _uRecvFile_2_FILEEOF;
            }
            if (ruheader->Control != UHEADER_CONTROL_FILEDATA){
                //正确性校验：判断是否是文件数据
                continue;
            }
            //报文正确，是文件数据
            if (ruheader->AckNum == this->ubuff->endptr){
                fwrite((char *)(recvbuff + sizeof(uheader_t)), ruheader->DataLen, 1, fp);
                this->ubuff->write((char *)(recvbuff + sizeof(uheader_t)), ruheader->DataLen);
                this->ubuff->get((char *)(recvbuff + sizeof(uheader_t)), ruheader->DataLen);
                this->recvTimer = 0;
            }
            this->usendBuff->setInitPtr(ruheader->SeqNum);
            suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
            suheader->AckNum = this->usendBuff->curptr; //远端序列号
            suheader->HeadLen = 16;
            suheader->Control = UHEADER_CONTROL_ACK;
            suheader->Window = this->windowLen;
            suheader->CheckSum = 0;
            suheader->DataLen = uint16_t(0); 
            suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
            this->_uconnSendTo(sendbuff, this->gramLen);
        }
        usleep(UCONN_FSM_USLEEP_TIME);
    }
    if (this->recvTimer >= UCONN_FSM_TIME_OUT*10){
        printf("接收失败\n");
        fclose(fp);
        return -1;
    }
    //文件结尾
    _uRecvFile_2_FILEEOF:
    printf("文件接收成功\n");
    this->usendBuff->setInitPtr(ruheader->SeqNum);
    suheader->SeqNum = this->ubuff->endptr; //本机期待收到的下一个序列号
    suheader->AckNum = this->usendBuff->curptr; //远端序列号
    suheader->HeadLen = 16;
    suheader->Control = UHEADER_CONTROL_ACK;
    suheader->Window = this->windowLen;
    suheader->CheckSum = 0;
    suheader->DataLen = uint16_t(0);
    suheader->CheckSum = this->_uconnComputeCheckSum(sendbuff, this->gramLen);
    this->_uconnSendTo(sendbuff, this->gramLen); //发送三次以避免丢失
    this->_uconnSendTo(sendbuff, this->gramLen); //发送三次以避免丢失
    this->_uconnSendTo(sendbuff, this->gramLen); //发送三次以避免丢失
    fclose(fp);
    free(sendbuff);
    free(recvbuff);
    free(blockbuff);
    return 1;
}

int Uconn::isClosed(){
    return this->commonState == Closed ? 1 : 0;
}

int Uconn::isOpen(){
    return this->commonState == Closed ? 0 : 1;
}

int Uconn::uconnClose(){
    return this->_uconnClose_1();
}

Uconn::~Uconn(){
    //pthread_join(this->recvThreadID, NULL);
    delete this->ubuff;
    delete this->usendBuff;
    delete this->remoteSeq;
}
