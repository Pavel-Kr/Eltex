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
    char msg[]="Multicast message";
    struct sockaddr_in broadcast;
    broadcast.sin_family=AF_INET;
    broadcast.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    if(inet_aton("224.0.0.1",&broadcast.sin_addr)==0){
        perror("inet_aton");
        pthread_exit(NULL);
    }
    broadcast.sin_port=htons(7777);
    while(1){
        if(sendto(socket,msg,sizeof(msg),0,(struct sockaddr*)&broadcast,sizeof(broadcast))==-1){
            perror("sendto");
        }
        printf("Send multicast message\n");
        sleep(1);
    }
}
int main(){
    int broadcast=socket(AF_INET,SOCK_DGRAM,0);
    if(broadcast==-1){
        perror("socket");
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