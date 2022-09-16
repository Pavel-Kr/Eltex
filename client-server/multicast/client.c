#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
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
    int mult_socket=socket(AF_INET,SOCK_DGRAM,0);
    if(mult_socket==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct ip_mreqn multicast;
    if(inet_aton("224.0.0.1",&multicast.imr_multiaddr)==0){
        perror("inet_aton");
        exit(EXIT_FAILURE);
    }
    multicast.imr_address.s_addr=INADDR_ANY;
    multicast.imr_ifindex=0;
    if(setsockopt(mult_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&multicast,sizeof(multicast))==-1){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in mult;
    memset(&mult,0,sizeof(mult));
    mult.sin_family=AF_INET;
    mult.sin_addr.s_addr=INADDR_ANY;
    mult.sin_port=htons(7777);
    if(bind(mult_socket,(struct sockaddr*)&mult,sizeof(struct sockaddr_in))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    pthread_t receiver;
    if(pthread_create(&receiver,NULL,recv_msg,&mult_socket)==-1){
        perror("create thread");
        exit(EXIT_FAILURE);
    }
    getchar();
    pthread_cancel(receiver);
    close(mult_socket);
    exit(EXIT_SUCCESS);
}