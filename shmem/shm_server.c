#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <semaphore.h>
struct user{
    char name[50];
    pid_t id;
};
struct message{
    char text[1000];
    struct user sender;
};
struct shmem{
    char name[20];
    int desc;
    sem_t* sem;
    char sem_name[20];
    struct message* msg_ptr;
};
struct user_env{
    struct user user;
    struct shmem stoc;
    struct shmem ctos;
    pthread_t tid;
    char deleted;
};
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
struct user_env* users;
int users_count=0,active_users_count=0,size=0;
int send_srv_message(char* shmem_name, char* sem_name, char* msg){
    struct message ans;
    ans.sender.id=0;
    int ans_mem=shm_open(shmem_name,O_RDWR,S_IRWXU);
    if(ans_mem==-1){
        perror("shm_open ans");
        return -1;
        //continue;
    }
    struct message* ans_ptr=mmap(NULL,sizeof(struct message),PROT_WRITE|PROT_READ,MAP_SHARED,ans_mem,0);
    if(ans_ptr==MAP_FAILED){
        perror("mmap ans");
        return -1;
        //continue;
    }
    sem_t* ans_sem=sem_open(sem_name,O_RDWR);
    if(ans_sem==SEM_FAILED){
          perror("sem_open ans");
          return -1;
          //continue;
    }
    strcpy(ans.text,msg);
    *ans_ptr=ans;
    sem_post(ans_sem);
    sem_close(ans_sem);
    if(munmap(ans_ptr,sizeof(struct message))==-1){
        perror("munmap ans");
        return -1;
    }
    return 0;
}
void* user_chat(void* arg){
    struct shmem ctos=*(struct shmem*)arg;
    ctos.msg_ptr=mmap(NULL,sizeof(struct message),PROT_WRITE|PROT_READ,MAP_SHARED,ctos.desc,0);
    if(ctos.msg_ptr==MAP_FAILED){
        perror("mmap pthread");
        pthread_exit(NULL);
    }
    while(1){
        struct message msg;
        sem_wait(ctos.sem);
        msg=*ctos.msg_ptr;
        for(int i=0;i<users_count;i++){
            *users[i].stoc.msg_ptr=msg;
            sem_post(users[i].stoc.sem);
        }
    }
}
void free_shmem(struct shmem shm){
    munmap(shm.msg_ptr,sizeof(struct message));
    sem_unlink(shm.sem_name);
    shm_unlink(shm.name);
}
void* service(void* arg){
    struct shmem service=*(struct shmem*)arg;
    struct message* msg_mem=mmap(NULL,sizeof(struct message),PROT_READ,MAP_SHARED,service.desc,0);
    if(msg_mem==MAP_FAILED){
        perror("mmap service");
        exit(EXIT_FAILURE);
    }
    users=malloc(sizeof(struct user_env)*10);
    size=10;
    while(1){
        struct message msg;
        sem_wait(service.sem);
        msg=*msg_mem;
        printf("%s(%d): %s\n",msg.sender.name,msg.sender.id,msg.text);
        int msg_len;
        char** msg_split=split(msg.text,&msg_len);
        if(strcmp(msg_split[0],"New")==0){
            struct user_env new_user;
            char stoc_name[20];
            char ctos_name[20];
            sprintf(stoc_name,"/shm%dstoc",msg.sender.id);
            sprintf(ctos_name,"/shm%dctos",msg.sender.id);
            int stoc=shm_open(stoc_name,O_RDWR|O_CREAT,S_IRWXU);
            if(stoc==-1){
                perror("shm_open stoc");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            if(ftruncate(stoc,sizeof(struct message))==-1){
                perror("ftruncate stoc");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            else{
                printf("%s created\n",stoc_name);
            }
            int ctos=shm_open(ctos_name,O_RDWR|O_CREAT,S_IRWXU);
            if(ctos==-1){
                perror("shm_open ctos");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            if(ftruncate(ctos,sizeof(struct message))==-1){
                perror("ftruncate ctos");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            else{
                printf("%s created\n",ctos_name);
            }
            char stoc_sem_name[20]="/sem";
            char ctos_sem_name[20];
            sprintf(stoc_sem_name,"/sem%dstoc",msg.sender.id);
            sprintf(ctos_sem_name,"/sem%dctos",msg.sender.id);
            sem_t* stoc_sem=sem_open(stoc_sem_name,O_RDWR|O_CREAT,S_IRWXU,0);
            if(stoc_sem==SEM_FAILED){
                perror("sem_open stoc");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            sem_t* ctos_sem=sem_open(ctos_sem_name,O_RDWR|O_CREAT,S_IRWXU,0);
            if(ctos_sem==SEM_FAILED){
                perror("sem_open ctos");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            new_user.user=msg.sender;
            strcpy(new_user.stoc.name,stoc_name);
            new_user.stoc.desc=stoc;
            strcpy(new_user.stoc.sem_name,stoc_sem_name);
            new_user.stoc.sem=stoc_sem;
            new_user.stoc.msg_ptr=mmap(NULL,sizeof(struct message),PROT_WRITE|PROT_READ,MAP_SHARED,new_user.stoc.desc,0);
            if(new_user.stoc.msg_ptr==MAP_FAILED){
                perror("mmap stoc");
                continue;
            }

            strcpy(new_user.ctos.name,ctos_name);
            new_user.ctos.desc=ctos;
            strcpy(new_user.ctos.sem_name,ctos_sem_name);
            new_user.ctos.sem=ctos_sem;

            pthread_t user_handler;
            if(pthread_create(&user_handler,NULL,user_chat,&new_user.ctos)!=0){
                perror("pthread_create");
                send_srv_message(msg_split[1],msg_split[2],"Nack");
                continue;
            }
            new_user.tid=user_handler;
            new_user.deleted=0;
            char ans_text[100];
            sprintf(ans_text,"Ack %s %s %s %s",stoc_name,ctos_name,stoc_sem_name,ctos_sem_name);
            send_srv_message(msg_split[1],msg_split[2],ans_text);
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
        else if(strcmp(msg_split[0],"Exit")==0){
            for(int i=0;i<users_count;i++){
                if(users[i].user.id==msg.sender.id){
                    users[i].deleted=1;
                    active_users_count--;
                    pthread_cancel(users[i].tid);
                    free_shmem(users[i].stoc);
                    free_shmem(users[i].ctos);
                }
            }
        }
        for(int i=0;i<msg_len;i++){
            free(msg_split[i]);
        }
        free(msg_split);
    }
}
int main(){
    int srv_mem=shm_open("/service",O_RDWR|O_CREAT,S_IRWXU);
    if(srv_mem==-1){
        perror("shm_open service");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(srv_mem,sizeof(struct message))==-1){
        perror("ftruncate service");
        exit(EXIT_FAILURE);
    }
    sem_t* srv_sem=sem_open("/service_sem",O_RDWR|O_CREAT,S_IRWXU,0);
    if(srv_sem==SEM_FAILED){
        perror("sem_open srv_sem");
        exit(EXIT_FAILURE);
    }
    struct shmem srv;
    strcpy(srv.name,"/service");
    srv.desc=srv_mem;
    srv.sem=srv_sem;
    strcpy(srv.sem_name,"/service_sem");
    pthread_t srv_handler;
    pthread_create(&srv_handler,NULL,service,&srv);
    while(1){
        char c=getchar();
        if(c=='e'){
            shm_unlink("/service");
            shm_unlink("/service_sem");
            pthread_cancel(srv_handler);
            for(int i=0;i<users_count;i++){
                if(!users[i].deleted){
                    send_srv_message(users[i].stoc.name,users[i].stoc.sem_name,"Dead");
                    pthread_cancel(users[i].tid);
                    free_shmem(users[i].stoc);
                    free_shmem(users[i].ctos);
                }
            }
            exit(EXIT_SUCCESS);
        }
        else if(c=='u'){
            for(int i=0;i<users_count;i++){
                printf("%s(%d)\t%d\n",users[i].user.name,users[i].user.id,users[i].deleted);
            }
            printf("Total users count: %d\nActive users count: %d\n",users_count,active_users_count);
        }
        while(getchar()!='\n');
    }
}