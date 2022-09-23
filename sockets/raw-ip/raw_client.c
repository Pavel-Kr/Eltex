#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
struct ip_header
{
    char ver_IHL;//version=4,header length=5;
    char DS;//always 0;
    short length;//auto fill
    short id;//auto fill if 0
    short flags_offset;//flags=0,frag,end; offset of fragment in full packet
    char TTL;//time to live
    char protocol;//transport protocol
    short checksum;//auto fill
    int source_ip;//auto fill if 0
    int dest_ip;
};

void ip_udp_strcat(char* dest, char* ip_header, char* udp_header, char* text){
    int i,j;
    for(i=0;i<20;i++){
        dest[i]=ip_header[i];
    }
    for(j=0;j<8;j++){
        dest[i++]=udp_header[j];
    }
    j=0;
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
    int flag=1;
    if(setsockopt(raw_fd,IPPROTO_IP,IP_HDRINCL,&flag,sizeof(flag))==-1){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv;
    serv.sin_family=AF_INET;
    serv.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    serv.sin_port=htons(4000);
    char str[10]="Hello!";
    char msg[50];
    short int udp_header[4];
    udp_header[0]=htons(7777);//Source port
    udp_header[1]=htons(4000);//Destination port
    udp_header[2]=htons(18);//Length
    udp_header[3]=0;//Checksum
    struct ip_header ip;
    ip.ver_IHL=(4<<4)|5;
    ip.DS=0;
    ip.id=0;
    ip.flags_offset=0;
    ip.TTL=255;
    ip.protocol=17;
    ip.source_ip=0;
    ip.dest_ip=htonl(INADDR_LOOPBACK);
    ip_udp_strcat(msg,(char*)&ip,(char*)udp_header,str);
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