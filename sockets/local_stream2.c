#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
int main(){
    int sockfd=socket(AF_LOCAL,SOCK_STREAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un serv;
    serv.sun_family=AF_LOCAL;
    strcpy(serv.sun_path,"sock_ls");
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