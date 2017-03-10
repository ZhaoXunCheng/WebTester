#include"Signal.h"
#include"Socket.h"
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>

#include<string.h>  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Already included in Socket.h. Pay attention to it.

#define MAXHOSTLEN 1024
#define MAXURLLEN 1024
#define MAXREQUESTLEN 2048
#define MAXPORTLEN 8
#define MAXPATHLEN 1024

volatile bool tickingTime = false;
int succeed = 0;
int failed = 0;
int bytes = 0;

int clients = 1;
int testTime = 30;

int mypipe[2];
char url[MAXURLLEN];
char host[MAXHOSTLEN];
char request[MAXREQUESTLEN];
char port[MAXPORTLEN];
char path[MAXPATHLEN];
unsigned short portnum = 80;  //default value

static void Test();
static void build_request(const char* url);

static void alarm_handler(int signo){
    tickingTime = true;
}

int main(){

    TIME:printf("Please enter the time you want to test:\n");

    if(scanf("%d", &testTime) != 1){
        printf("Invalid input.\n");
        goto TIME;
    }

    if(testTime == 0){
        testTime = 30;
    }

    CLIENT:printf("Please enter the number of clients you want to simiulate:\n");

    if(scanf("%d", &clients) != 1){
        printf("Invalid input.\n");
        goto CLIENT;
    }

    if(clients == 0){
        clients = 1;
    }

    URL:printf("Please enter the url you want to test:\n");

    if(scanf("%s", url) != 1 || url[strlen(url) - 1] != '/'){
        printf("Invalid input.\n");
        goto URL;
    }

    build_request(url);

    Test();

    return 0;
}

void build_request(const char* url){
    size_t i = 0, j = 0;
    size_t url_len = strlen(url);

    while(url[j] != ':' && url[j] != '/' && j < url_len){
        host[i] = url[j];
        ++i;
        ++j;
    }

    host[i] = '\0';

    if(url[j] == ':'){  //port number
        i = 0;

        ++j;

        while(url[j] != '/' && j < url_len){
            port[i] = url[j];
            ++i;
            ++j;
        }

        port[i] = '\0';

        if((portnum = (unsigned short)atoi(port)) <= 0){
            portnum = 80;
        }
    }
    //  path
    i = 0;

    while(j < url_len){
        path[i] = url[j];
        ++i;
        ++j;
    }

    path[i] = '\0';

    strcpy(request, "GET ");

    strcat(request, path);

    strcat(request, " HTTP/1.1\r\n");

    strcat(request, "Host: ");

    strcat(request, host);

    strcat(request, "\r\n");

    strcat(request, "Connection: close\r\n");

    strcat(request, "User-Agent: Mozilla/5.0\r\n");

    strcat(request, "\r\n");

    return;
}

void Test(){
    int _succeed, _failed, _bytes;
    pid_t pid;
    char buf[1500];

    if(pipe(mypipe) != 0){
        printf("error in creating pipe.\n");
        printf("error value: %d\n", errno);
        printf("error message: %s\n", strerror(errno));
    }

    Signal(SIGALRM, alarm_handler);

    int i;

    for(i = 0; i < clients; ++i){
        if((pid = fork()) <= 0){
            sleep(3);
            break;
        }
    }

    if(pid < 0){
        fprintf(stderr, "error in creating child process no.%d\n", i);
        exit(1);
    }

    if(pid == 0){  //child process
        alarm(testTime);
        int sockfd;
        int len = strlen(request);
        int n;

        START:while(tickingTime == false){

            if((sockfd = Socket(host, portnum)) < 0){
                if(errno != EINTR){
                    ++failed;
                }
                continue;
            }

            if(send(sockfd, request, len, 0) != len){
                if(errno != EINTR){
                    ++failed;
                }
                close(sockfd);
                continue;
            }

            if(shutdown(sockfd, SHUT_WR) != 0){
                if(errno != EINTR){
                    ++failed;
                }
                close(sockfd);
                continue;
            }

            while(true){
                n = recv(sockfd, buf, sizeof(buf), 0);
                if(n < 0){
                    if(errno != EINTR){
                        ++failed;
                    }
                    close(sockfd);
                    goto START;
                }
                else if(n == 0){
                    break;
                }
                else{  //n > 0
                    bytes += n;
                }
            }

            ++succeed;

            close(sockfd);
        }

        FILE* fp = fdopen(mypipe[1], "w");

        fprintf(fp, "%d %d %d ", succeed, failed, bytes);

        fclose(fp);

        exit(0);
    }

    if(pid > 0){  //father process

        FILE* fp = fdopen(mypipe[0], "r");

        while(clients--){

            if(fscanf(fp, "%d %d %d", &_succeed, &_failed, &_bytes) != 3){
                fprintf(stderr, "some of the child processes failed.\n");
                exit(2);
            }

            succeed += _succeed;

            failed += _failed;

            bytes += _bytes;
        }

        printf("test result:\n");

        printf("succeed: %d\n", succeed);

        printf("failed: %d\n", failed);

        printf("total bytes: %d\n", bytes);

        return;
    }
}

















