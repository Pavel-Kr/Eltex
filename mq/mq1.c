#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int main(){
    pid_t pid=fork();
    if(pid==0){
        if(execl("./mq2","",NULL)==-1){
            perror("exec");
            exit(EXIT_FAILURE);
        }
    }
    else{
        struct mq_attr attr;
        attr.mq_flags=0;
        attr.mq_msgsize=10;
        attr.mq_maxmsg=10;
        attr.mq_curmsgs=0;
        mqd_t queue[2];
        queue[0]=mq_open("/queue0",O_WRONLY|O_CREAT|O_EXCL,S_IRWXU,&attr);
        if(queue[0]==-1){
            if(errno!=EEXIST){
                perror("mq_open0");
                exit(EXIT_FAILURE);
            }
            else{
                queue[0]=mq_open("/queue0",O_WRONLY);
            }
        }
        char str[20]="Hello!";
        if(mq_send(queue[0],str,strlen(str)+1,1)==-1){
            perror("mq_send1");
            //exit(EXIT_FAILURE);
        }
        queue[1]=mq_open("/queue1",O_RDONLY|O_CREAT|O_EXCL,S_IRWXU,&attr);
        if(queue[1]==-1){
            if(errno!=EEXIST){
                perror("mq_open1");
                exit(EXIT_FAILURE);
            }
            else{
                queue[1]=mq_open("/queue1",O_RDONLY);
            }
        }
        char ans[20];
        if(mq_receive(queue[1],ans,20,NULL)==-1){
            perror("mq_receive1");
            //exit(EXIT_FAILURE);
        }
        printf("%s\n",ans);
        mq_close(queue[0]);
        mq_close(queue[1]);
        wait(NULL);
        mq_unlink("/queue0");
        mq_unlink("/queue1");
        exit(EXIT_SUCCESS);
    }
}