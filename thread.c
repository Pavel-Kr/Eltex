#include <pthread.h>
#include <stdio.h>
int a=0;
void* func(void* arg){
    for(int i=0;i<1000000;i++){
        int tmp=a;
        tmp++;
        a=tmp;
    }
    return 0;
}
int main(){
    pthread_t tid[5];
    void** status;
    for(int i=0;i<5;i++){
        pthread_create(&tid[i],NULL,func,NULL);
    }
    for(int i=0;i<5;i++){
        pthread_join(tid[i],NULL);
    }
    printf("%d\n",a);
}