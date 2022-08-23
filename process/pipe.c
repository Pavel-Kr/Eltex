#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
int main(){
    int pipefd[2];
    if(pipe(pipefd)==-1){
        perror("Creating pipe error");
        exit(EXIT_FAILURE);
    }
    pid_t pid=fork();
    if(pid==-1){
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    if(pid==0){
        close(pipefd[1]);
        char ch;
        while(read(pipefd[0],&ch,1)>0){
            printf("%c",ch);
        }
        printf("\n");
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else{
        close(pipefd[0]);
        char* str="Data to send to another process";
        write(pipefd[1],str,strlen(str));
        close(pipefd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}