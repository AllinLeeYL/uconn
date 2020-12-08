#include "ubuff.h"

Useq::Useq(){
    this->seq = 0;
}

Useq::~Useq(){
    ;
}

Useq::Useq(int _size_){
    this->size = _size_;
}

Useq Useq::operator+(const Useq & _useq_) const{
    Useq useq;
    useq.size = this->size <= _useq_.size ? this->size : _useq_.size;
    for (useq.size = this->seq + _useq_.seq; useq.seq >= useq.size; useq.seq = useq.seq - useq.size){
        ;
    }
    return useq;
}

Useq Useq::operator+(uint32_t _n_) const{
    Useq useq;
    useq.size = this->size;
    for (useq.seq = this->seq + _n_; useq.seq >= useq.size; useq.seq = useq.seq - useq.size){
        ;
    }
    return useq;
}

int Useq::operator=(const Useq & _useq_){
    this->size = _useq_.size;
    this->seq = _useq_.seq;
    return 1;
}

int Useq::operator<(const Useq & _useq_) const{
    int distance = 0;
    if (_useq_.seq == this->seq){
        return 0;
    }
    else if (this->seq < _useq_.seq){
        distance = this->seq + this->size - _useq_.seq;
    }
    else{
        distance = this->seq - _useq_.seq;
    }
    if (distance > int(this->size / 2)){
        return 1;
    }
    else{
        return 0;
    }
}

int Useq::operator<(uint32_t _n_) const{
    int distance = 0;
    if (_n_ == this->seq){
        return 0;
    }
    else if (this->seq < _n_){
        distance = this->seq + this->size - _n_;
    }
    else{
        distance = this->seq - _n_;
    }
    if (distance > int(this->size / 2)){
        return 1;
    }
    else{
        return 0;
    }
}

int Useq::operator>(const Useq & _useq_) const{
    int distance = 0;
    if (_useq_.seq == this->seq){
        return 0;
    }
    else{
        return *this < _useq_ ? 0 : 1;
    }
}

int Useq::operator>(uint32_t _n_) const{
    int distance = 0;
    if (_n_ == this->seq){
        return 0;
    }
    else{
        return *this < _n_ ? 0 : 1;
    }
}

Ubuff::Ubuff(){
    this->curptr = 0;
    this->endptr = 0;
    this->buffSize = 65536;
    this->buff = (char *)malloc(this->buffSize);
    memset(this->buff, 0, this->buffSize);
}

Ubuff::Ubuff(uint32_t _buffSize_){
    this->curptr = 0;
    this->endptr = 0;
    this->buffSize = _buffSize_;
    this->buff = (char *)malloc(this->buffSize);
    memset(this->buff, 0, this->buffSize);
}

int Ubuff::setInitPtr(uint32_t _initPtr_){
    if (_initPtr_ >= this->buffSize){
        return -1;
    }
    this->curptr = _initPtr_;
    this->endptr = _initPtr_;
    return 0;
}

int Ubuff::size(){
    return this->buffSize - this->remainSize() - 1;
}

int Ubuff::remainSize(){
    if (this->curptr > this->endptr){
        return this->curptr - this->endptr - 1;
    }
    else{
        return this->buffSize - this->endptr + this->curptr - 1;
    }
}

int Ubuff::write(char * _buff_, uint32_t _len_){
    if (this->remainSize() < _len_){
        return -1;
    }
    for (int i = 0; i < _len_; i = i + 1){
        this->buff[this->endptr] = _buff_[i];
        this->endptr = this->endptr + 1;
        if (this->endptr >= this->buffSize){
            this->endptr = 0;
        }
    }
    return _len_;
}

int Ubuff::read(char * _buff_, uint32_t _len_){
    _len_ = this->size() <= _len_ ? this->size() : _len_;
    //读取数据
    for (int i = 0; i < _len_; i = i + 1){
        if ((this->curptr + i) < this->buffSize){
            _buff_[i] = this->buff[this->curptr + i];
        }
        else{
            _buff_[i] = this->buff[this->curptr + i - this->buffSize];
        }
    }
    //不移动指针
    return _len_;
}

int Ubuff::get(char * _buff_, uint32_t _len_){
    _len_ = this->size() <= _len_ ? this->size() : _len_;
    //读取数据
    for (int i = 0; i < _len_; i = i + 1){
        if ((this->curptr + i) < this->buffSize){
            _buff_[i] = this->buff[this->curptr + i];
        }
        else{
            _buff_[i] = this->buff[this->curptr + i - this->buffSize];
        }
    }
    //移动指针
    if (this->curptr + _len_ < this->buffSize){
        this->curptr = this->curptr + _len_;
    }
    else {
        this->curptr = this->curptr + _len_ - this->buffSize;
    }
    return _len_;
}

int Ubuff::operator<(uint32_t _n_) const{
    int distance = 0;
    if (_n_ == this->endptr){
        return 0;
    }
    else if (this->endptr < _n_){
        distance = this->endptr + this->buffSize - _n_;
    }
    else{
        distance = this->endptr - _n_;
    }
    if (distance > int(this->buffSize / 2)){
        return 1;
    }
    else{
        return 0;
    }
}

int Ubuff::operator>(uint32_t _n_) const{
    int distance = 0;
    if (_n_ == this->endptr){
        return 0;
    }
    else{
        return *this < _n_ ? 0 : 1;
    }
}

Ubuff::~Ubuff(){
    free(this->buff);
}