#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
int main(){
    int sockfd=socket(AF_LOCAL,SOCK_STREAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    /*if(fchmod(sockfd,S_IRWXU)==-1){
        perror("chmod");
        exit(EXIT_FAILURE);
    }*/
    struct sockaddr_un serv;
    serv.sun_family=AF_LOCAL;
    strcpy(serv.sun_path,"sock_ls");
    if(bind(sockfd,(struct sockaddr*)&serv,sizeof(struct sockaddr_un))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd,1)==-1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un client;
    socklen_t cl_size=sizeof(client);
    int clientfd=accept(sockfd,(struct sockaddr*)&client,&cl_size);
    if(clientfd==-1){
        perror("accept");
        exit(EXIT_FAILURE);
    }
    char str[10]="Hello!";
    if(send(clientfd,str,sizeof(str),0)==-1){
        perror("send1");
        exit(EXIT_FAILURE);
    }
    if(recv(clientfd,str,sizeof(str),0)==-1){
        perror("recv1");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",str);
    close(clientfd);
    close(sockfd);
    unlink("sock_ls");
    exit(EXIT_SUCCESS);
}