#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
struct user{
    char name[50];
    pid_t id;
}user;
struct message{
    char msg[1000];
    struct user sender;
};
struct queues{
    mqd_t stoc;
    mqd_t service;
}queues;
char* itos(int num){
    char* str=malloc(10);
    int i=0;
    while(num>0){
        str[i]=num%10+'0';
        num/=10;
        i++;
    }
    for(int j=0;j<i/2;j++){
        char tmp=str[j];
        str[j]=str[i-j-1];
        str[i-j-1]=tmp;
    }
    str[i]=0;
    return str;
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
void* send(void* arg){
    mqd_t ctos=mq_open("/ctos",O_WRONLY);
    if(ctos==-1){
        perror("mq_open ctos");
        exit(EXIT_FAILURE);
    }
    char text[1000];
    while(1){
        fgets(text,1000,stdin);
        if(strcmp(text,"/exit\n")==0){
            mq_close(ctos);
            struct message msg;
            strcpy(msg.msg,"Exit");
            msg.sender=user;
            mq_send(queues.service,(char*)&msg,sizeof(msg),1);
            pthread_exit(0);
        }
        struct message msg;
        strcpy(msg.msg,text);
        msg.sender=user;
        mq_send(ctos,(char*)&msg,sizeof(msg),1);
    }
}
void* receive(void* arg){
    char* stoc_name=(char*)arg;
    mqd_t stoc=mq_open(stoc_name,O_RDONLY);
    if(stoc==-1){
        perror("mq_open stoc");
        printf("%s\n",stoc_name);
        exit(EXIT_FAILURE);
    }
    while(1){
        struct message msg;
        int prio;
        mq_receive(stoc,(char*)&msg,sizeof(msg),&prio);
        if(msg.sender.id!=0){
            printf("%s: %s",msg.sender.name,msg.msg);
        }
        else{
            if(strcmp(msg.msg,"Dead")==0){
                printf("Server is not available now\n");
            }
        }
    }
}
int main(){
    printf("Enter your name: ");
    fgets(user.name,50,stdin);
    for(int i=0;i<50;i++){
        if(user.name[i]=='\n'){
            user.name[i]=0;
            break;
        }
    }
    user.id=getpid();
    mqd_t srv_queue=mq_open("/service",O_RDWR);
    if(srv_queue==-1){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    queues.service=srv_queue;
    char ans_queue_name[20]="/service";
    char* id_str=itos(user.id);
    strcat(ans_queue_name,id_str);
    free(id_str);
    struct mq_attr attr;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=sizeof(struct message);
    mqd_t ans_queue=mq_open(ans_queue_name,O_RDONLY|O_CREAT,S_IRWXU,&attr);
    if(ans_queue==-1){
        perror("mq_open ans");
        exit(EXIT_FAILURE);
    }
    char new_msg[25]="New ";
    strcat(new_msg,ans_queue_name);
    struct message new;
    strcpy(new.msg,new_msg);
    new.sender=user;
    mq_send(srv_queue,(char*)&new,sizeof(new),1);
    int prio;
    mq_receive(ans_queue,(char*)&new,sizeof(new),&prio);
    int msg_len;
    char** queue_names=split(new.msg,&msg_len);
    char stoc_name[20];
    strcpy(stoc_name,queue_names[1]);
    mq_unlink(ans_queue_name);
    pthread_t sender,receiver;
    pthread_create(&sender,NULL,send,NULL);
    pthread_create(&receiver,NULL,receive,stoc_name);
    pthread_join(sender,NULL);
    pthread_cancel(receiver);
    for(int i=0;i<msg_len;i++){
        free(queue_names[i]);
    }
    free(queue_names);
    exit(EXIT_SUCCESS);
}