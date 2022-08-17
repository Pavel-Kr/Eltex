#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <malloc.h>
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
        while(str[i]!=' '&&str[i]!='\n'){
            res[j][k]=str[i];
            k++;
            i++;
        }
        i++;
    }
    *n=spaces+1;
    res[j]=NULL;
    return res;
}
int main(){
    char cmd[200];
    int len=0;
    int status;
    while(1){
        printf("$ ");
        fgets(cmd,199,stdin);
        char** args=split(cmd,&len);
        pid_t pid=fork();
        if(pid==0){
            if(execvp(args[0],args)==-1){
                perror("Cannot find command");
            }
        }
        else{
            wait(&status);
        }
        for(int i=0;i<len;i++){
            free(args[i]);
        }
        free(args);
    }
}