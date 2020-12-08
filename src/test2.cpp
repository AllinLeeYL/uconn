#include "Config.h"

#define PORT 30001

int main(){
    Uconn * uconn = new Uconn();
    struct sockaddr_in servaddr;
    bzero(&servaddr , sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
    uconn->ubindAddr((struct sockaddr *)&servaddr);
    char buff[60];

    while (uconn->uconnAccept() < 0){
        printf("failure\n");
    }
    printf("success\n");
    while (uconn->isOpen()){
        int n = uconn->uGetBuff(buff, 60);
        if (n > 0){
            printf("%s\n", buff);
        }
    }
    delete uconn;
}