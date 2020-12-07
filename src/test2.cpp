#include "Config.h"

#define PORT 30001

int main(){
    Uconn * uconn = new Uconn();
    struct sockaddr_in servaddr;
    bzero(&servaddr , sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT+1);
    uconn->ubindAddr((struct sockaddr *)&servaddr);
    inet_pton(AF_INET , "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(PORT);
    while (uconn->uconnBuild((struct sockaddr *)&servaddr) < 0){
        printf("failure\n");
    }
    printf("success\n");
    delete uconn;
}