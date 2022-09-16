#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
void* recv_msg(void* arg){
    int socket=*(int*)arg;
    char msg[100];
    struct sockaddr_in sender;
    socklen_t snd_size=sizeof(sender);
    while(1){
        if(recvfrom(socket,msg,sizeof(msg),0,(struct sockaddr*)&sender,&snd_size)==-1){
            perror("recvfrom");
        }
        printf("%s\n",msg+28);
    }
}
int main(){
    int raw_socket=socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
    if(raw_socket==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    pthread_t receiver;
    if(pthread_create(&receiver,NULL,recv_msg,&raw_socket)==-1){
        perror("create thread");
        exit(EXIT_FAILURE);
    }
    getchar();
    pthread_cancel(receiver);
    close(raw_socket);
    exit(EXIT_SUCCESS);
}