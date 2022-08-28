#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <semaphore.h>
struct user{
    char name[50];
    pid_t id;
}user;
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
}service;
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
    struct shmem ctos=*(struct shmem*)arg;
    ctos.desc=shm_open(ctos.name,O_RDWR,S_IRWXU);
    if(ctos.desc==-1){
        perror("shm_open ctos");
        pthread_exit((void*)-1);
    }
    ctos.msg_ptr=mmap(NULL,sizeof(struct message),PROT_WRITE,MAP_SHARED,ctos.desc,0);
    if(ctos.msg_ptr==MAP_FAILED){
        perror("mmap ctos");
        pthread_exit((void*)-1);
    }
    ctos.sem=sem_open(ctos.sem_name,O_RDWR);
    if(ctos.sem==SEM_FAILED){
        perror("sem_open ctos");
        pthread_exit((void*)-1);
    }
    char text[1000];
    while(1){
        fgets(text,1000,stdin);
        if(strcmp(text,"/exit\n")==0){
            struct message msg;
            strcpy(msg.text,"Exit");
            msg.sender=user;
            *service.msg_ptr=msg;
            sem_post(service.sem);
            pthread_exit(0);
        }
        struct message msg;
        strcpy(msg.text,text);
        msg.sender=user;
        *ctos.msg_ptr=msg;
        sem_post(ctos.sem);
    }
}
void* receive(void* arg){
    struct shmem stoc=*(struct shmem*)arg;
    stoc.desc=shm_open(stoc.name,O_RDWR,S_IRWXU);
    if(stoc.desc==-1){
        perror("shm_open stoc");
        pthread_exit((void*)-1);
    }
    stoc.msg_ptr=mmap(NULL,sizeof(struct message),PROT_READ,MAP_SHARED,stoc.desc,0);
    if(stoc.msg_ptr==MAP_FAILED){
        perror("mmap stoc");
        pthread_exit((void*)-1);
    }
    stoc.sem=sem_open(stoc.sem_name,O_RDWR);
    if(stoc.sem==SEM_FAILED){
        perror("sem_open stoc");
        pthread_exit((void*)-1);
    }
    while(1){
        struct message msg;
        sem_wait(stoc.sem);
        msg=*stoc.msg_ptr;
        if(msg.sender.id!=0){
            printf("%s: %s",msg.sender.name,msg.text);
        }
        else{
            if(strcmp(msg.text,"Dead")==0){
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
    int srv_mem=shm_open("/service",O_RDWR,S_IRWXU);
    if(srv_mem==-1){
        perror("shm_open service");
        exit(EXIT_FAILURE);
    }
    strcpy(service.name,"/service");
    service.desc=srv_mem;
    struct message* new_ptr=mmap(NULL,sizeof(struct message),PROT_WRITE,MAP_SHARED,srv_mem,0);
    if(new_ptr==MAP_FAILED){
        perror("mmap service");
        exit(EXIT_FAILURE);
    }
    service.msg_ptr=new_ptr;
    sem_t* srv_sem=sem_open("/service_sem",O_RDWR);
    if(srv_sem==SEM_FAILED){
        perror("sem_open service");
        exit(EXIT_FAILURE);
    }
    strcpy(service.sem_name,"/service_sem");
    service.sem=srv_sem;
    char ans_mem_name[20];
    sprintf(ans_mem_name,"/service%d",user.id);
    printf("%s\n",ans_mem_name);
    int ans_mem=shm_open(ans_mem_name,O_RDWR|O_CREAT,S_IRWXU);
    if(ans_mem==-1){
        perror("shm_open ans");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(ans_mem,sizeof(struct message))==-1){
        perror("ftruncate ans_mem");
        exit(EXIT_FAILURE);
    }
    struct message* ans_ptr=mmap(NULL,sizeof(struct message),PROT_READ,MAP_SHARED,ans_mem,0);
    if(ans_ptr==MAP_FAILED){
        perror("mmap ans");
        exit(EXIT_FAILURE);
    }
    char ans_sem_name[20];
    sprintf(ans_sem_name,"/sem%d",user.id);
    printf("%s\n",ans_sem_name);
    sem_t* ans_sem=sem_open(ans_sem_name,O_RDWR|O_CREAT,S_IRWXU,0);
    if(ans_sem==SEM_FAILED){
        perror("sem_open ans");
        exit(EXIT_FAILURE);
    }
    char new_msg[50];
    sprintf(new_msg,"New %s %s",ans_mem_name,ans_sem_name);
    struct message new;
    strcpy(new.text,new_msg);
    new.sender=user;
    *new_ptr=new;
    sem_post(srv_sem);
    struct message ans;
    sem_wait(ans_sem);
    ans=*ans_ptr;
    sem_unlink(ans_sem_name);
    shm_unlink(ans_mem_name);
    int msg_len;
    char** shm_names=split(ans.text,&msg_len);
    struct shmem stoc,ctos;
    if(strcmp(shm_names[0],"Nack")==0){
        goto free;
    }
    strcpy(stoc.name,shm_names[1]);
    strcpy(ctos.name,shm_names[2]);
    strcpy(stoc.sem_name,shm_names[3]);
    strcpy(ctos.sem_name,shm_names[4]);
    pthread_t sender,receiver;
    pthread_create(&sender,NULL,send,&ctos);
    pthread_create(&receiver,NULL,receive,&stoc);
    pthread_join(sender,NULL);
    pthread_cancel(receiver);
    free:
    munmap(new_ptr,sizeof(struct message));
    sem_close(srv_sem);
    sem_unlink(ans_sem_name);
    shm_unlink(ans_mem_name);
    for(int i=0;i<msg_len;i++){
        free(shm_names[i]);
    }
    free(shm_names);
    exit(EXIT_SUCCESS);
}