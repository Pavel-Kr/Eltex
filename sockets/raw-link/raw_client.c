#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
struct link_header{
    char dest_mac[6];
    char src_mac[6];
    short type;
};
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

void link_ip_udp_strcat(char* dest, char* link, char* ip_header, char* udp_header, char* text){
    int i=0,j;
    for(j=0;j<14;j++){
        dest[i++]=link[j];
    }
    for(j=0;j<20;j++){
        dest[i++]=ip_header[j];
    }
    for(j=0;j<8;j++){
        dest[i++]=udp_header[j];
    }
    j=0;
    do{
        dest[i++]=text[j++];
    }while(text[j]!=0);
}
short CRC16(char* buf){
    int csum=0;
    short* ptr=(short*)buf;
    for(int i=0;i<10;i++){
        csum+=*ptr;
        ptr++;
    }
    while((csum>>16)!=0){
        short tmp=csum>>16;
        csum=(csum&0xFFFF)+tmp;
    }
    csum=~csum;
    return csum;
}
int main(){
    int raw_fd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(raw_fd==-1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_ll serv;
    memset(&serv,0,sizeof(serv));
    serv.sll_family=AF_PACKET;
    //serv.sll_protocol=htons(ETH_P_ALL);
    serv.sll_ifindex=if_nametoindex("enp0s8");
    if(serv.sll_ifindex==0){
        perror("nametoindex");
        exit(EXIT_FAILURE);
    }
    serv.sll_halen=6;
    char str[10]="Hello!";
    char msg[100];
    short int udp_header[4];
    udp_header[0]=htons(7777);//Source port
    udp_header[1]=htons(4000);//Destination port
    udp_header[2]=htons(sizeof(str)+8);//Length
    udp_header[3]=0;//Checksum
    struct ip_header ip;
    ip.ver_IHL=(4<<4)|5;
    ip.DS=0;
    ip.id=15;
    ip.checksum=0;
    ip.flags_offset=0;
    ip.TTL=255;
    ip.protocol=17;
    ip.length=htons(sizeof(str)+28);
    ip.source_ip=inet_addr("10.0.2.4");
    ip.dest_ip=inet_addr("10.0.2.5");
    ip.checksum=CRC16((char*)&ip);
    struct link_header link;
    memset(&link,0,sizeof(link));
    link.dest_mac[0]=serv.sll_addr[0]=0x08;//08:00:27:d4:9d:02
    link.dest_mac[1]=serv.sll_addr[1]=0x00;
    link.dest_mac[2]=serv.sll_addr[2]=0x27;
    link.dest_mac[3]=serv.sll_addr[3]=0xd4;
    link.dest_mac[4]=serv.sll_addr[4]=0x9d;
    link.dest_mac[5]=serv.sll_addr[5]=0x02;
    link.src_mac[0]=0x08;
    link.src_mac[1]=0x00;
    link.src_mac[2]=0x27;
    link.src_mac[3]=0xae;
    link.src_mac[4]=0x3a;
    link.src_mac[5]=0xcb;
    link.type=htons(0x0800);//IP
    link_ip_udp_strcat(msg,(char*)&link,(char*)&ip,(char*)udp_header,str);
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
        short* dest_port=(short*)(ans+36);
        if(htons(*dest_port)==7777){
            printf("%s\n",ans+42);
        }
    }
}