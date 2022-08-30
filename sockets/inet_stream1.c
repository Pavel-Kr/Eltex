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
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv;
    memset(&serv,0,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_port=htons(4000);
    serv.sin_addr.s_addr=INADDR_LOOPBACK;
    /*if(inet_aton("10.0.2.20",&serv.sin_addr)==0){
        perror("inet_aton");
        exit(EXIT_FAILURE);
    }*/
    if(bind(sockfd,(struct sockaddr*)&serv,sizeof(struct sockaddr_in))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd,1)==-1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in client;
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
    unlink("sock_is");
    exit(EXIT_SUCCESS);
}