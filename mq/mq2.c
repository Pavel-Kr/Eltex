#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
int main(){
    struct mq_attr attr;
    attr.mq_flags=0;
    attr.mq_msgsize=10;
    attr.mq_maxmsg=100;
    attr.mq_curmsgs=0;
    mqd_t queue[2];
    char str[20];
    queue[0]=mq_open("/queue0",O_RDONLY|O_CREAT|O_EXCL,S_IRWXU,&attr);
    if(queue[0]==-1){
        if(errno!=EEXIST){
            perror("mq_open0");
            exit(EXIT_FAILURE);
        }
        else{
            queue[0]=mq_open("/queue0",O_RDONLY);
        }
    }
    if(mq_receive(queue[0],str,20,NULL)==-1){
        perror("mq_receive2");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",str);
    queue[1]=mq_open("/queue1",O_WRONLY|O_CREAT|O_EXCL,S_IRWXU,&attr);
    if(queue[1]==-1){
        if(errno!=EEXIST){
            perror("mq_open1");
            exit(EXIT_FAILURE);
        }
        else{
            queue[1]=mq_open("/queue1",O_WRONLY);
        }
    }
    strcpy(str,"Hi!");
    if(mq_send(queue[1],str,strlen(str),1)==-1){
        perror("mq_send2");
        exit(EXIT_FAILURE);
    }
    mq_close(queue[0]);
    mq_close(queue[1]);
}