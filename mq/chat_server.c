#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
struct user{
    char name[50];
    pid_t id;
};
struct message{
    char msg[1000];
    struct user sender;
};
struct mq{
    struct user user;
    mqd_t stoc;
    char stoc_name[20];
    char deleted;
};
char** split(char* str){
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
    return res;
}
struct mq* users;
int users_count=0,active_users_count=0;
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
void* service(void* arg){
    mqd_t srv_queue=*(mqd_t*)arg;
    users=malloc(sizeof(struct mq)*10);
    while(1){
        struct message msg;
        int prio;
        mq_receive(srv_queue,(char*)&msg,sizeof(struct message),&prio);
        printf("%s(%d): %s %d\n",msg.sender.name,msg.sender.id,msg.msg,prio);
        char** msg_split=split(msg.msg);
        if(strcmp(msg_split[0],"New")==0){
            char stoc_name[20]="/queue";
            char* id_str=itos(msg.sender.id);
            strcat(stoc_name,id_str);
            free(id_str);
            strcat(stoc_name,"stoc");
            struct mq_attr stoc_attr={0,10,sizeof(struct message),0};
            mqd_t stoc=mq_open(stoc_name,O_WRONLY|O_CREAT,S_IRWXU,&stoc_attr);
            if(stoc==-1){
                perror("mq_open stoc");
                continue;
            }
            else{
                printf("%s created\n",stoc_name);
            }
            mqd_t ans_queue=mq_open(msg_split[1],O_WRONLY);
            struct message ans;
            strcpy(ans.msg,"Ack ");
            strcat(ans.msg,stoc_name);
            mq_send(ans_queue,(char*)&ans,sizeof(struct message),1);
            mq_close(ans_queue);
            struct mq mq_user;
            mq_user.user=msg.sender;
            mq_user.stoc=stoc;
            strcpy(mq_user.stoc_name,stoc_name);
            mq_user.deleted=0;
            for(int i=0;i<users_count;i++){
                if(users[i].deleted==1){
                    users[i]=mq_user;
                    active_users_count++;
                }
            }
            int size=sizeof(*users)/sizeof(struct mq);
            if(active_users_count>=size){
                users=realloc(users,size*2);
            }
            users[users_count++]=mq_user;
            active_users_count++;
        }
        else if(strcmp(msg_split[0],"Exit")==0){
            for(int i=0;i<users_count;i++){
                if(users[i].user.id==msg.sender.id){
                    users[i].deleted=1;
                    active_users_count--;
                    mq_close(users[i].stoc);
                    mq_unlink(users[i].stoc_name);
                }
            }
        }
    }
}
void* user_chat(void* arg){
    mqd_t ctos=*(mqd_t*)arg;
    while(1){
        struct message msg;
        int prio;
        mq_receive(ctos,(char*)&msg,sizeof(msg),&prio);
        for(int i=0;i<users_count;i++){
            mq_send(users[i].stoc,(char*)&msg,sizeof(msg),1);
        }
    }
}
int main(){
    struct mq_attr attr;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=sizeof(struct message);
    mqd_t srv_queue=mq_open("/service",O_RDWR|O_CREAT,S_IRWXU,&attr);
    if(srv_queue==-1){
        perror("mq_open service");
        exit(EXIT_FAILURE);
    }
    mqd_t ctos=mq_open("/ctos",O_RDONLY|O_CREAT,S_IRWXU,&attr);
    if(srv_queue==-1){
        perror("mq_open ctos");
        exit(EXIT_FAILURE);
    }
    pthread_t srv_handler;
    pthread_create(&srv_handler,NULL,service,&srv_queue);
    pthread_t user_handler;
    pthread_create(&user_handler,NULL,user_chat,&ctos);
    while(1){
        if(getchar()=='e'){
            mq_unlink("/service");
            mq_unlink("/ctos");
            pthread_cancel(srv_handler);
            pthread_cancel(user_handler);
            for(int i=0;i<users_count;i++){
                if(!users[i].deleted){
                    struct message msg;
                    strcpy(msg.msg,"Dead");
                    msg.sender.id=0;
                    mq_send(users[i].stoc,(char*)&msg,sizeof(msg),1);
                    mq_unlink(users[i].stoc_name);
                }
            }
            exit(EXIT_SUCCESS);
        }
        while(getchar()!='\n');
    }
}