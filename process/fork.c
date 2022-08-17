#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(){
    pid_t pid=fork();
    int status;
    if(pid==-1){
        perror("Error: cannot create new process!");
        exit(EXIT_FAILURE);
    }
    if(pid==0){
        printf("Child pid = %d\nChild ppid = %d\n",getpid(),getppid());
        exit(EXIT_SUCCESS);
    }
    else{
        printf("Parent pid = %d\nParent ppid = %d\n",getpid(),getppid());
        wait(&status);
        exit(EXIT_SUCCESS);
    }
}