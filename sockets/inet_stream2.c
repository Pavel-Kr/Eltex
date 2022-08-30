#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main(){
    int sockfd=socket(AF_LOCAL,SOCK_STREAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv;
    serv.sin_family=AF_INET;
    serv.sin_port=htons(3000);
    if(inet_aton("192.168.1.1",&serv.sin_addr)==0){
        perror("inet_aton");
        exit(EXIT_FAILURE);
    }
    socklen_t serv_size=sizeof(serv);
    if(connect(sockfd,(struct sockaddr*)&serv,serv_size)==-1){
        perror("connect");
        exit(EXIT_FAILURE);
    }
    char str[10];
    if(recv(sockfd,str,sizeof(str),0)==-1){
        perror("recv");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",str);
    strcpy(str,"Hi!");
    if(send(sockfd,str,sizeof(str),0)==-1){
        perror("send");
        exit(EXIT_FAILURE);
    }
    close(sockfd);
    exit(EXIT_SUCCESS);
}