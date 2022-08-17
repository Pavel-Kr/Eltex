#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(int argc,char* argv[]){
    pid_t pid=fork();
    int status;
    if(pid==-1){
        perror("Error: cannot create new process!");
        exit(EXIT_FAILURE);
    }
    if(pid==0){
        if(execve("./proxy",argv,NULL)==-1){
            perror("Exec error!");
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("Parent process\n");
        wait(&status);
        exit(EXIT_SUCCESS);
    }
}