#include "Config.h"

#define PORT 30001

int uconntest(){
    Uconn * uconn = new Uconn(0);
    char buff[65536];
    struct sockaddr_in servaddr;

    bzero(&servaddr , sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
    uconn->ubindAddr((struct sockaddr *)&servaddr);

    while (uconn->uconnAccept() < 0){
        ;
    }
    printf("连接成功\n");
    while (uconn->uRecvFile(buff) < 0){
        ;
    }
    printf("文件名：%s\n", buff);
}

int filetest(){
    
}

int main(){
    uconntest();
}