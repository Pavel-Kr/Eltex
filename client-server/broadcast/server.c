#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
void* send_msg(void* arg){
    int socket=*(int*)arg;
    char msg[]="Broadcast message";
    struct sockaddr_in broadcast;
    broadcast.sin_family=AF_INET;
    broadcast.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    broadcast.sin_port=htons(7777);
    while(1){
        if(sendto(socket,msg,sizeof(msg),0,(struct sockaddr*)&broadcast,sizeof(broadcast))==-1){
            perror("sendto");
        }
        printf("Send broadcast message\n");
        sleep(1);
    }
}
int main(){
    int broadcast=socket(AF_INET,SOCK_DGRAM,0);
    if(broadcast==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int flag=1;
    if(setsockopt(broadcast,SOL_SOCKET,SO_BROADCAST,&flag,sizeof(flag))==-1){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    pthread_t sender;
    if(pthread_create(&sender,NULL,send_msg,&broadcast)==-1){
        perror("create thread");
        exit(EXIT_FAILURE);
    }
    getchar();
    pthread_cancel(sender);
    close(broadcast);
    exit(EXIT_SUCCESS);
}