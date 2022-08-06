#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <malloc.h>
#include <string.h>
void input(int* a, int* b){
    printf("Введите первое число: ");
    if(0==scanf("%d",a)){
        perror("Error!");
        exit(1);
    }
    printf("Введите второе число: ");
    if(0==scanf("%d",b)){
        perror("Error!");
        exit(1);
    }
}
struct lib_desc{
    char path[50];
    char header[20];
    void* handle;
    int (*function)(int,int);
};
int main(){
    setlocale(LC_ALL,"rus");
    struct lib_desc* libs=NULL;
    int size=0;
    while(1){
        printf("Введите имя библиотеки: ");
        char path[50];
        if(0==scanf("%s",path)){
            perror("Error!");
            continue;
        }
        while (getchar()!='\n');
        void* lib=dlopen(path,RTLD_LAZY);
        if(!lib){
            printf("%s\n",dlerror());
            continue;
        }
        char* header=dlsym(lib,"header");
        if(!header){
            printf("Cannot find header!\n");
            dlclose(lib);
            continue;
        }
        int (*func)(int,int)=dlsym(lib,"operation");
        if(!func){
            printf("Cannot find function int operation(int,int)!\n");
            dlclose(lib);
            continue;
        }
        libs=realloc(libs,(size+1)*sizeof(struct lib_desc));
        if(!libs){
            perror("Out of memory!");
            exit(EXIT_FAILURE);
        }
        strcpy(libs[size].path,path);
        strcpy(libs[size].header,header);
        libs[size].handle=lib;
        libs[size].function=func;
        size++;
        printf("Продолжить ввод? y/n: ");
        char select;
        if(0==scanf("%c",&select)){
            perror("Error!");
            exit(EXIT_FAILURE);
        }
        while (getchar()!='\n');
        if(select!='y') break;
    }
    int select;
    while(1){
        for(int i=0;i<size;i++){
            printf("%d - %s, ",i+1,libs[i].header);
        }
        printf("любая другая кнопка - выход: ");
        if(0==scanf("%d",&select)) return 0;
        else if(select<=0||select>size) return 0;
        int a,b,res;
        input(&a,&b);
        res=libs[select-1].function(a,b);
        printf("Результат: %d\n",res);
        while(getchar()!='\n');
    }
    for(int i=0;i<size;i++){
        dlclose(libs[i].handle);
    }
    free(libs);
    return 0;
}