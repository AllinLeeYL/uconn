#include "ftransfer.h"

ftransfer::ftransfer(){
    this->fp = NULL;
    memset(this->filename, 0, FTRANSFER_FILENAME_LEN);
    this->fileSize = 0;
    this->cur = 0;
}

int ftransfer::ftropen(char * _filename_, char * _strategy_){
    strcpy(this->filename, _filename_);
    this->fp = fopen(this->filename, _strategy_);
    if (this->fp == NULL){
        return -1;
    }
    return 0;
}

int ftransfer::ftrSend(char * _filename_, struct sockaddr * _addr_){

}

int ftransfer::ftrRecv(char * _directory_, char * _filename_){

}