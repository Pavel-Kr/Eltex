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
    unlink("sock_ld");
    int sockfd=socket(AF_LOCAL,SOCK_DGRAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un serv;
    serv.sun_family=AF_LOCAL;
    strcpy(serv.sun_path,"sock_ld");
    if(bind(sockfd,(struct sockaddr*)&serv,sizeof(struct sockaddr_un))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_un client;
    socklen_t cl_size=sizeof(client);
    char str[10];
    if(recvfrom(sockfd,str,sizeof(str),0,(struct sockaddr*)&client,&cl_size)==-1){
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",str);
    strcpy(str,"Hi!");
    if(sendto(sockfd,str,sizeof(str),0,(struct sockaddr*)&client,cl_size)==-1){
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    close(sockfd);
    unlink("sock_ld");
    exit(EXIT_SUCCESS);
}