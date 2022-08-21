#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int shop[5];
pthread_mutex_t m1=PTHREAD_MUTEX_INITIALIZER;
void* buy(void* arg){
    int demand=9500+rand()%1000;
    while(demand>0){
        int index=rand()%5;
        pthread_mutex_lock(&m1);
        int min=(shop[index]<demand)?shop[index]:demand;
        shop[index]-=min;
        int remain=shop[index];
        pthread_mutex_unlock(&m1);
        demand-=min;
        printf("Customer %d bought %d goods in shop %d, %d demand remain\n",*(int*)arg,min,index+1,demand);
        sleep(2);
    }
    return 0;
}
void* load(void* arg){
    while(1){
        int index=rand()%5;
        pthread_mutex_lock(&m1);
        shop[index]+=500;
        pthread_mutex_unlock(&m1);
        printf("Loaded 500 goods in shop %d\n",index+1);
        sleep(1);
    }
}
int main(){
    for(int i=0;i<5;i++){
        shop[i]=900+rand()%200;
    }
    pthread_t customer[3];
    int num[3];
    for(int i=0;i<3;i++){
        num[i]=i+1;
        pthread_create(&customer[i],NULL,buy,&num[i]);
    }
    pthread_t loader;
    pthread_create(&loader,NULL,load,NULL);
    for(int i=0;i<3;i++){
        pthread_join(customer[i],NULL);
    }
    pthread_cancel(loader);
    exit(EXIT_SUCCESS);
}