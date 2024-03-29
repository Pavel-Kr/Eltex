#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
int main(){
    unlink("sock_cl");
    int sockfd=socket(AF_LOCAL,SOCK_DGRAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un client;
    client.sun_family=AF_LOCAL;
    strcpy(client.sun_path,"sock_cl");
    if(bind(sockfd,(struct sockaddr*)&client,sizeof(client))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un serv;
    serv.sun_family=AF_LOCAL;
    strcpy(serv.sun_path,"sock_ld");
    socklen_t serv_size=sizeof(serv);
    char str[10]="Hello!";
    if(sendto(sockfd,str,sizeof(str),0,(struct sockaddr*)&serv,serv_size)==-1){
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    if(recvfrom(sockfd,str,sizeof(str),0,(struct sockaddr*)&serv,&serv_size)==-1){
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",str);
    close(sockfd);
    unlink("sock_cl");
    exit(EXIT_SUCCESS);
}