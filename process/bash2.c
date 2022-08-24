#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <malloc.h>
#include <string.h>
char** split(char* str, int* n, int* pipes){
    int i=0;
    int spaces=0;
    int str_flag=0;
    while(str[i]!=0){
        if(str[i]=='"') str_flag=1-str_flag;
        if(str[i]==' '&&(!str_flag)) spaces++;
        i++;
    }
    char** res=malloc((spaces+1)*sizeof(char*));
    i=0;
    int j;
    for(j=0;j<=spaces;j++){
        res[j]=malloc(50);
        int k=0;
        while(str[i]!='\n'){
            if(str[i]=='"'){
                str_flag=1-str_flag;
                i++;
                continue;
            }
            else if(str[i]==' '&&(!str_flag)){
                break;
            }
            else if(str[i]=='|') (*pipes)++;
            res[j][k]=str[i];
            k++;
            i++;
        }
        res[j][k]=0;
        i++;
    }
    *n=spaces+1;
    //res[j]=NULL;
    return res;
}
int main(){
    char cmd[200];
    int len=0;
    int status;
    int pipes;
    char** args;
    while(1){
        pipes=0;
        printf("$ ");
        fgets(cmd,199,stdin);
        args=split(cmd,&len,&pipes);
        int j=0;
        int pipefd[2];
        if(pipe(pipefd)==-1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        for(int i=0;i<=pipes;i++){
            char** opts=malloc((len+1)*sizeof(char*));
            int k=0;
            while(j<len){
                if(strcmp(args[j],"|")==0){
                    j++;
                    break;
                }
                opts[k]=malloc(50);
                strcpy(opts[k],args[j]);
                //printf("%s ",opts[k]);
                //free(opts[k]);
                k++;
                j++;
            }
            opts[k]=NULL;
            pid_t pid=fork();
            if(pid==-1){
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if(pid==0){
                if(i>0){
                    if(dup2(pipefd[0],0)==-1){
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                if(i<pipes){
                    if(dup2(pipefd[1],1)==-1){
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                if(execvp(opts[0],opts)==-1){
                    perror("Cannot find command");
                }
            }
            else{
                wait(&status);
                char eof=EOF;
                write(pipefd[1],&eof,1);
            }
            for(int a=0;a<k;a++){
                free(opts[a]);
            }
            free(opts);
            //printf("\n");
        }
        for(int i=0;i<len;i++){
            free(args[i]);
        }
        free(args);
    }
}