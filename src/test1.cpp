#include "Config.h"
#include <sys/time.h>

#define PORT 30001

int uconntest(){
    Uconn * uconn = new Uconn(0);
    char buff[65536] = "fuck!";
    struct sockaddr_in servaddr;

    bzero(&servaddr , sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT+1);
    uconn->ubindAddr((struct sockaddr *)&servaddr);

    inet_pton(AF_INET , "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(PORT);

    while (uconn->uconnBuild((struct sockaddr *)&servaddr) < 0){
        ;
    }
    printf("连接成功\n");
    time_t start,stop;
    start = time(NULL);
    FILE * fp = fopen("./test/1.jpg", "rb");
    if (uconn->uSendFile(fp, "1.jpg") > 0){
        printf("文件名：%s\n", "1.jpg");
    }
    
    fclose(fp);
    stop = time(NULL);
    printf("发送用时:%ld\n",(stop-start));
}

int sleeptest(){
    struct timeval start, end;
    gettimeofday(&start, NULL);
    sleepnsec(2000000000);
    gettimeofday(&end, NULL);
    printf("%d\n", (end.tv_sec-start.tv_sec) * 1000000 + end.tv_usec-start.tv_usec);
}

int main(){
    sleeptest();
}