#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(){
    int fd=open("buffer.buf",O_RDONLY);
    if(fd==-1){
        perror("open mkfifo2");
        exit(EXIT_FAILURE);
    }
    char ch;
    while (read(fd,&ch,1)>0)
    {
        printf("%c",ch);
    }
    printf("\n");
    exit(EXIT_SUCCESS);
}