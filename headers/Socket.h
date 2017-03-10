#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>

#ifndef Socket_h_
#define Socket_h_

int Socket(const char* host, int port){
    int sockfd;
    in_addr** pptr;
    hostent* hp;
    unsigned long inaddr;
    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    if((inaddr = inet_addr(host)) != INADDR_NONE){  //host is an ip address
        server_addr.sin_addr.s_addr = inaddr;
        if(connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0){
            return -1;
        }
        else{
            return sockfd;
        }
    }
    else{  //host is a host name
        if((hp = gethostbyname(host)) == NULL){
            return -1;
        }
        else{
            pptr = (in_addr**)hp->h_addr_list;
            while(*pptr != NULL){
                memcpy(&server_addr.sin_addr, *pptr, sizeof(in_addr));
                if(connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0){
                    ++pptr;
                }
                else{
                    return sockfd;
                }
            }
            return -1;
        }
    }
}

#endif


















