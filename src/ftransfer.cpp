#include "ftransfer.h"

ftransfer::ftransfer(){
    memset(this->filename, 0, FTRANSFER_BLOCK_SIZE);
    this->fp = NULL;
    this->fsp = NULL;
    this->uconn = new Uconn();
    this->timer = 0;
}

ftransfer::~ftransfer(){
    delete this->uconn;
}

void ftransfer::bindAddr(struct sockaddr * _servaddr_){
    uconn->ubindAddr(_servaddr_);
}

void ftransfer::bindRemoteAddr(struct sockaddr * _servaddr_){
    bstrcpy((char *)&(this->remoteAddr), (char *)_servaddr_, sizeof(struct sockaddr));
}

int ftransfer::open(char * _filename_, char * _strategy_){
    strcpy(this->filename, _filename_);
    this->fp = fopen(this->filename, _strategy_);
    if (this->fp == NULL){
        return -1;
    }
    this->fsp = fsplit(this->fp, FTRANSFER_BLOCK_SIZE);
    return 1;
}

int ftransfer::close(){
    fclose(this->fp);
    free(this->fsp);
}

int ftransfer::Send(char * _filename_, struct sockaddr * _addr_){
    this->timer = 0;
    char buff[sizeof(ftrheader_t) + FTRANSFER_BLOCK_SIZE];
    ftrheader_t * ftrheader;
    ftrheader = (ftrheader_t *)buff;
    while (this->timer < FTRANSFER_TRY_TIMES){
        if (this->uconn->uconnBuild(&(this->remoteAddr)) >= 0){
            /*文件名*/
            ftrheader->protocol = FTRHEADER_PROTOCOL;
            ftrheader->control = FTRHEADER_CONTROL_FILENAME;
            ftrheader->length = sizeof(ftrheader_t) + FTRANSFER_BLOCK_SIZE;
            bstrcpy((char *)(buff + sizeof(ftrheader_t)), this->filename, FTRANSFER_BLOCK_SIZE);
            this->uconn->uSendBuff(buff, sizeof(ftrheader_t) + FTRANSFER_BLOCK_SIZE);
            /*文件*/
            ftrheader->protocol = FTRHEADER_PROTOCOL;
            ftrheader->control = FTRHEADER_CONTROL_FILEDATA;
            ftrheader->length = sizeof(ftrheader_t) + FTRANSFER_BLOCK_SIZE;
            int n = fsplt_next((char *)(buff + sizeof(ftrheader_t)), this->fsp, FTRANSFER_BLOCK_SIZE);
            while (n > 0){
                this->uconn->uSendBuff(buff, sizeof(ftrheader_t) + FTRANSFER_BLOCK_SIZE);
                n = fsplt_next((char *)(buff + sizeof(ftrheader_t)), this->fsp, FTRANSFER_BLOCK_SIZE);
            }
            /*文件结束*/
            ftrheader->protocol = FTRHEADER_PROTOCOL;
            ftrheader->control = FTRHEADER_CONTROL_FILEEOF;
            ftrheader->length = sizeof(ftrheader_t) + FTRANSFER_BLOCK_SIZE;
            this->uconn->uSendBuff(buff, sizeof(ftrheader_t));
            return 1;
        }
    }
    return -1;
}

int ftransfer::Recv(char * _directory_, char * _filename_){
    
}