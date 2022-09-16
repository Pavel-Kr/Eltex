#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
void* recv_msg(void* arg){
    int socket=*(int*)arg;
    char msg[20];
    struct sockaddr_in sender;
    socklen_t snd_size=sizeof(sender);
    while(1){
        if(recvfrom(socket,msg,sizeof(msg),0,(struct sockaddr*)&sender,&snd_size)==-1){
            perror("recvfrom");
        }
        printf("%s\n",msg);
    }
}
int main(){
    int br_socket=socket(AF_INET,SOCK_DGRAM,0);
    if(br_socket==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv;
    memset(&serv,0,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    serv.sin_port=htons(7777);
    if(bind(br_socket,(struct sockaddr*)&serv,sizeof(struct sockaddr_in))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    pthread_t receiver;
    if(pthread_create(&receiver,NULL,recv_msg,&br_socket)==-1){
        perror("create thread");
        exit(EXIT_FAILURE);
    }
    getchar();
    pthread_cancel(receiver);
    close(br_socket);
    exit(EXIT_SUCCESS);
}