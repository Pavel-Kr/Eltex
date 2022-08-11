#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
int main(){
    int file=open("file_io.txt",O_CREAT|O_WRONLY,S_IRWXU);
    if(file<0){
        perror("Cannot open file!");
        exit(EXIT_FAILURE);
    }
    char str[]="Reverse string";
    write(file,str,strlen(str));
    close(file);
    file=open("file_io.txt",O_RDONLY);
    if(file<0){
        perror("Cannot open file!");
        exit(EXIT_FAILURE);
    }
    struct stat filestats;
    fstat(file,&filestats);
    int size=filestats.st_size;
    char buf[size+1];
    buf[size]=0;
    for(int i=size-1;i>=0;i--){
        if(-1==read(file,&buf[i],1)){
            perror("Error at reading file!");
            exit(EXIT_FAILURE);
        }
    }
    printf("%s\n",buf);
    close(file);
}