#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/epoll.h>

#define TCP 0
#define UDP 1
#define MAX_EVENTS 10

struct user{
    char name[50];
    struct sockaddr_in endp;
};
struct message{
    char text[1000];
    struct user sender;
};
struct user_env{
    struct user user;
    int user_fd;
    char deleted;
    char protocol;
};
struct user_handler{
    pthread_t tid;
    mqd_t queue_fd;
}*handlers;
struct sockets{
    int tcp;
    int udp;
};
int usercmp(struct user u1, struct user u2){
    if(u1.endp.sin_addr.s_addr==u2.endp.sin_addr.s_addr && u1.endp.sin_port==u2.endp.sin_port) return 1;
    else return 0;
}
char** split(char* str, int* n){
    int i=0;
    int spaces=0;
    while(str[i]!=0){
        if(str[i]==' ') spaces++;
        i++;
    }
    char** res=malloc((spaces+1)*sizeof(char*));
    i=0;
    int j;
    for(j=0;j<=spaces;j++){
        res[j]=malloc(50);
        int k=0;
        while(str[i]!='\0'){
            if(str[i]==' '){
                break;
            }
            res[j][k]=str[i];
            k++;
            i++;
        }
        res[j][k]=0;
        i++;
    }
    *n=spaces+1;
    return res;
}
int mp_send(struct user_env *client, struct message *msg){
    if(client->protocol==TCP){
        return send(client->user_fd,msg,sizeof(struct message),0);
    }
    else if(client->protocol==UDP){
        return sendto(client->user_fd,msg,sizeof(struct message),0,(struct sockaddr*)&client->user.endp,sizeof(struct sockaddr_in));
    }
    else return -1;
}
int mp_recv(struct user_env *client, struct message *msg){
    if(client->protocol==TCP){
        return recv(client->user_fd,msg,sizeof(struct message),0);
    }
    else if(client->protocol==UDP){
        socklen_t cl_size=sizeof(struct sockaddr_in);
        return recvfrom(client->user_fd,msg,sizeof(struct message),0,(struct sockaddr*)&client->user.endp,&cl_size);
    }
    else return -1;
}
struct user_env* users;
int users_count=0,active_users_count=0,size=0;
void* add_user(void* arg){
    struct user_env new_user=*(struct user_env*)arg;
    
    pthread_exit(NULL);
}
void* user_chat(void* arg){
    struct user_handler *handler=(struct user_handler*)arg;
    int queue=mq_open("/user_queue",O_RDONLY);
    if(queue==-1){
        perror("mq_open thread");
        pthread_exit(NULL);
    }
    handler->queue_fd=queue;
    while(1){
        struct user_env client;
        if(mq_receive(queue,(char*)&client,sizeof(client),NULL)==-1){
            perror("mq_receive thread");
            pthread_exit(NULL);
        }
        if(client.protocol==UDP){
            int udp_socket=socket(AF_INET,SOCK_DGRAM,0);
            if(udp_socket==-1){
                perror("udp socket thread");
                pthread_exit(NULL);
            }
            client.user_fd=udp_socket;
            for(int i=0;i<users_count;i++){
                if(usercmp(users[i].user,client.user)){
                    users[i].user_fd=udp_socket;
                    break;
                }
            }
        }
        struct message msg;
        strcpy(msg.text,"Ready");
        if(mp_send(&client,&msg)==-1){
            perror("mp_send Ready");
            pthread_exit(NULL);
        }
        while(1){
            if(mp_recv(&client,&msg)==-1){
                perror("mp_recv");
                continue;
            }
            int msg_len;
            char** msg_split=split(msg.text,&msg_len);
            if(strcmp(msg_split[0],"Name")==0){
                for(int i=0;i<users_count;i++){
                    if(usercmp(users[i].user,client.user)){
                        strcpy(users[i].user.name,msg_split[1]);
                    }
                }
                strcpy(client.user.name,msg_split[1]);
                printf("Name %s %s(%d)\n",msg_split[1],inet_ntoa(client.user.endp.sin_addr),ntohs(client.user.endp.sin_port));
            }
            else if(strcmp(msg_split[0],"Exit")==0){
                for(int i=0;i<users_count;i++){
                    if(usercmp(users[i].user,client.user)){
                        users[i].deleted=1;
                        active_users_count--;
                        close(users[i].user_fd);
                        printf("Exit %s %s(%d)\n",client.user.name,inet_ntoa(client.user.endp.sin_addr),ntohs(client.user.endp.sin_port));
                        break;
                    }
                }
                for(int i=0;i<msg_len;i++){
                    free(msg_split[i]);
                }
                free(msg_split);
                break;
            }
            else{
                msg.sender=client.user;
                for(int i=0;i<users_count;i++){
                    if(users[i].deleted==0){
                        if(mp_send(&users[i],&msg)==-1){
                            perror("mp_send user");
                        }
                    }
                }
            }
            for(int i=0;i<msg_len;i++){
                free(msg_split[i]);
            }
            free(msg_split);
        }
    }
}
void* service(void* arg){
    struct sockets sk=*(struct sockets*)arg;
    int tcp_socket=sk.tcp;
    int udp_socket=sk.udp;
    size=10;
    users=malloc(sizeof(struct user_env)*size);
    handlers=malloc(sizeof(struct user_handler)*size);
    struct mq_attr attr;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=sizeof(struct user_env);
    mqd_t queue=mq_open("/user_queue",O_WRONLY|O_CREAT,S_IRWXU,&attr);
    if(queue==-1){
        perror("mq_open queue");
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<size;i++){
        if(pthread_create(&handlers[i].tid,NULL,user_chat,&handlers[i])!=0){
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    int epfd=epoll_create1(0);
    if(epfd==-1){
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event tcp_event;
    tcp_event.data.fd=tcp_socket;
    tcp_event.events=EPOLLIN;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,tcp_socket,&tcp_event)==-1){
        perror("epoll_ctl tcp");
        exit(EXIT_FAILURE);
    }
    struct epoll_event udp_event;
    udp_event.data.fd=udp_socket;
    udp_event.events=EPOLLIN;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,udp_socket,&udp_event)==-1){
        perror("epoll_ctl udp");
        exit(EXIT_FAILURE);
    }
    while(1){
        struct sockaddr_in client;
        socklen_t cl_size=sizeof(client);
        struct epoll_event events[MAX_EVENTS];
        int evts=epoll_wait(epfd,events,MAX_EVENTS,-1);
        if(evts==-1){
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for(int n=0;n<evts;n++){
            struct user_env new_user;
            new_user.deleted=0;
            if(events[n].data.fd==tcp_socket){
                int client_fd=accept(tcp_socket,(struct sockaddr*)&client,&cl_size);
                if(client_fd==-1){
                    perror("accept");
                    continue;
                }
                printf("Tcp accept %s(%d)\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                new_user.user.endp=client;
                new_user.user_fd=client_fd;
                new_user.protocol=TCP;
            }
            else if(events[n].data.fd==udp_socket){
                struct message msg;
                struct sockaddr_in client;
                socklen_t cl_size=sizeof(client);
                if(recvfrom(udp_socket,&msg,sizeof(msg),0,(struct sockaddr*)&client,&cl_size)==-1){
                    perror("recvfrom service");
                    continue;
                }
                printf("Udp recv %s(%d)\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                new_user.user.endp=client;
                new_user.protocol=UDP;
            }
            if(mq_send(queue,(char*)&new_user,sizeof(new_user),0)==-1){
                perror("mq_send user");
                continue;
            }
            int i;
            for(i=0;i<users_count;i++){
                if(users[i].deleted==1){
                    users[i]=new_user;
                    active_users_count++;
                    break;
                }
            }
            if(i==users_count){
                if(active_users_count>=size){
                    users=realloc(users,size*2);
                    size*=2;
                }
                users[users_count++]=new_user;
                active_users_count++;
            }
        }
    }
}
int main(){
    int tcp_socket=socket(AF_INET,SOCK_STREAM,0);
    if(tcp_socket==-1){
        perror("tcp socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in tcp_serv;
    tcp_serv.sin_family=AF_INET;
    tcp_serv.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    int binded=0;
    for(int i=4000;i<5000;i++){
        tcp_serv.sin_port=htons(i);
        if(bind(tcp_socket,(struct sockaddr*)&tcp_serv,sizeof(tcp_serv))!=-1){
            printf("TCP port: %d\n",i);
            binded=1;
            break;
        }
    }
    if(!binded){
        tcp_serv.sin_port=htons(4000);
        if(bind(tcp_socket,(struct sockaddr*)&tcp_serv,sizeof(tcp_serv))!=-1){
            perror("bind");
            exit(EXIT_FAILURE);
        }
    }
    if(listen(tcp_socket,5)==-1){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int udp_socket=socket(AF_INET,SOCK_DGRAM,0);
    if(udp_socket==-1){
        perror("udp socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in udp_serv;
    udp_serv.sin_family=AF_INET;
    udp_serv.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    binded=0;
    for(int i=5000;i<6000;i++){
        udp_serv.sin_port=htons(i);
        if(bind(udp_socket,(struct sockaddr*)&udp_serv,sizeof(udp_serv))!=-1){
            printf("UDP port: %d\n",i);
            binded=1;
            break;
        }
    }
    if(!binded){
        udp_serv.sin_port=htons(5000);
        if(bind(udp_socket,(struct sockaddr*)&udp_serv,sizeof(udp_serv))!=-1){
            perror("bind");
            exit(EXIT_FAILURE);
        }
    }
    struct sockets sk;
    sk.tcp=tcp_socket;
    sk.udp=udp_socket;
    pthread_t srv_handler;
    pthread_create(&srv_handler,NULL,service,&sk);
    while(1){
        char c=getchar();
        if(c=='e'){
            pthread_cancel(srv_handler);
            for(int i=0;i<users_count;i++){
                if(!users[i].deleted){
                    struct message msg;
                    strcpy(msg.text,"Dead");
                    if(users[i].protocol==TCP) msg.sender.endp=tcp_serv;
                    else if(users[i].protocol==UDP) msg.sender.endp=udp_serv;
                    if(mp_send(&users[i],&msg)==-1){
                        perror("send");
                    }
                    close(users[i].user_fd);
                }
            }
            free(users);
            for(int i=0;i<size;i++){
                if(mq_close(handlers[i].queue_fd)==-1){
                    perror("mq_close");
                }
                pthread_cancel(handlers[i].tid);
            }
            if(mq_unlink("/user_queue")==-1){
                perror("mq_unlink");
            }
            exit(EXIT_SUCCESS);
        }
        else if(c=='u'){
            for(int i=0;i<users_count;i++){
                if(users[i].protocol==TCP)
                    printf("%s %s(%d)\t%d\tTCP\t%d\n",users[i].user.name,inet_ntoa(users[i].user.endp.sin_addr),ntohs(users[i].user.endp.sin_port),users[i].deleted,users[i].user_fd);
                else if(users[i].protocol==UDP)
                    printf("%s %s(%d)\t%d\tUDP\t%d\n",users[i].user.name,inet_ntoa(users[i].user.endp.sin_addr),ntohs(users[i].user.endp.sin_port),users[i].deleted,users[i].user_fd);
            }
            printf("Total users count: %d\nActive users count: %d\nThreads: %d\n",users_count,active_users_count,size);
        }
        while(getchar()!='\n');
    }
}