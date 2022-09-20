#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
void udp_strcat(char* dest, char* header, char* text){
    int i;
    for(i=0;i<8;i++){
        dest[i]=header[i];
    }
    int j=0;
    while(text[j]!=0){
        dest[i++]=text[j++];
    }
}
int main(){
    int raw_fd=socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
    if(raw_fd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv;
    serv.sin_family=AF_INET;
    serv.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    serv.sin_port=htons(4000);
    char str[10]="Hello!";
    char msg[18];
    short int udp_header[4];
    udp_header[0]=htons(7777);//Source port
    udp_header[1]=htons(4000);//Destination port
    udp_header[2]=htons(18);//Length
    udp_header[3]=0;//Checksum
    udp_strcat(msg,(char*)udp_header,str);
    if(sendto(raw_fd,msg,sizeof(msg),0,(struct sockaddr*)&serv,sizeof(serv))==-1){
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    char ans[100];
    socklen_t serv_size=sizeof(serv);
    while(1){
        if(recvfrom(raw_fd,ans,sizeof(ans),0,(struct sockaddr*)&serv,&serv_size)==-1){
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("%s\n",ans+28);
    }
}