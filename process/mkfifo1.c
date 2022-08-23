#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(){
    if(mkfifo("buffer.buf",S_IRWXU)==-1){
        if(errno!=EEXIST){
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }
    int fd=open("buffer.buf",O_WRONLY);
    if(fd==-1){
        perror("open mkfifo1");
        exit(EXIT_FAILURE);
    }
    char* str="Data to send to another process";
    write(fd,str,strlen(str));
    close(fd);
    exit(EXIT_SUCCESS);
}