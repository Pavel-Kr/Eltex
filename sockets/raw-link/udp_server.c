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
    int sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv;
    memset(&serv,0,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_port=htons(4000);
    serv.sin_addr.s_addr=inet_addr("10.0.2.5");
    if(bind(sockfd,(struct sockaddr*)&serv,sizeof(struct sockaddr_in))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in client;
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
    exit(EXIT_SUCCESS);
}