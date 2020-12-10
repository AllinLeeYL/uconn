#include "Config.h"

#define LOCAL_PORT 30001
#define WORK_STATUS_SEND 0
#define WORK_STATUS_RECV 1

Uconn * uconn;
struct sockaddr_in LOCAL_ADDR;
struct sockaddr_in REMOTE_ADDR;

char * INPUT_FILE_NAME;
int INPUT_WORK_STATUS;

char BUFF[2048];
time_t start,stop;

int _cutFilename(char * _fileBaseName_, char * _filepath_){
    for (int i = strlen(_filepath_) - 1; i >= 0; i = i - 1){
        if (_filepath_[i] == '/' || _filepath_[i] == '\\'){
            _fileBaseName_ = (char *)(_filepath_ + i + 1);
            return 0;
        }
    }
    return 0;
}

void print_help(){

}

void init(){
    uconn = new Uconn(1);
    bzero(&LOCAL_ADDR , sizeof(struct sockaddr_in));
    LOCAL_ADDR.sin_family = AF_INET;
	LOCAL_ADDR.sin_addr.s_addr = htonl(INADDR_ANY);

    bzero(&REMOTE_ADDR , sizeof(struct sockaddr_in));
    REMOTE_ADDR.sin_family = AF_INET;
    inet_pton(AF_INET , "127.0.0.1", &REMOTE_ADDR.sin_addr);
	REMOTE_ADDR.sin_port = htons(LOCAL_PORT);
}

void send(){
    FILE * fp = NULL;
    char * fileBaseName = NULL;

	LOCAL_ADDR.sin_port = htons(LOCAL_PORT+1);
    uconn->ubindAddr((struct sockaddr *)&LOCAL_ADDR);

    for (int i = 0; i < 3; i = i + 1){
        if (uconn->uconnBuild((struct sockaddr *)&REMOTE_ADDR) > 0){
            printf("连接建立\n");
            goto _send_SENDFILE;
        }
    }
    printf("发送失败：连接建立失败\n");

    _send_SENDFILE:
    fp = fopen(INPUT_FILE_NAME, "rb");
    if (fp == NULL){
        printf("文件\"%s\"不存在\n", INPUT_FILE_NAME);
        return;
    }
    _cutFilename(fileBaseName, INPUT_FILE_NAME);
    
    start = time(NULL);
    if (uconn->uSendFile(fp, fileBaseName) > 0){
        printf("文件\"%s\"发送成功\n", INPUT_FILE_NAME);
    }
    stop = time(NULL);
    printf("发送用时:%ld\n",(stop-start));

    fclose(fp);
}

void recv(){
	LOCAL_ADDR.sin_port = htons(LOCAL_PORT);
    uconn->ubindAddr((struct sockaddr *)&LOCAL_ADDR);

    while (1){
        while (uconn->uconnAccept() < 0){
            ;
        }
        printf("连接成功\n");
        if (uconn->uRecvFile(BUFF) < 0){
            printf("文件接收失败\n");
        }
        else{
            printf("文件\"%s\"接收成功\n", BUFF);
        }
    }
    return;
}

int main(int argc, char ** argv){
    if (argc == 1){
        //没有参数
        print_help();
        return 0;
    }
    for (int i = 1; i < argc; i = i + 1){
        //根据参数执行功能
        if (strcmp(argv[i], "-h") == 0){
            print_help();
            return 0;
        }
        else if (strcmp(argv[i], "-s") == 0){
            INPUT_WORK_STATUS = WORK_STATUS_SEND;
            continue;
        }
        else if (strcmp(argv[i], "-r") == 0){
            INPUT_WORK_STATUS = WORK_STATUS_RECV;
            continue;
        }
        else if (strcmp(argv[i], "-p") == 0){
            i = i + 1;
            if (i >= argc){
				perror("<param worng>:");
				print_help();
				exit(1);
			}
            REMOTE_ADDR.sin_port = htons((uint16_t)atoi(argv[i]));
            continue;
        }
        else if (strcmp(argv[i], "-a") == 0){
            i = i + 1;
            if (i >= argc){
				perror("<param worng>:");
				print_help();
				exit(1);
			}
            inet_pton(AF_INET , argv[i], &REMOTE_ADDR.sin_addr);
            continue;
        }
        else if (strcmp(argv[i], "-f") == 0){
            i = i + 1;
            if (i >= argc){
				perror("<param worng>:");
				print_help();
				exit(1);
			}
            INPUT_FILE_NAME = argv[i];
            continue;
        }
    }

    if (INPUT_WORK_STATUS == WORK_STATUS_SEND){
        send();
    }
    else{
        recv();
    }

    return 0;
}