#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include "calc.h"
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
int main(){
    setlocale(LC_ALL,"rus");
    char select;
    while(1){
        printf("1 - сложение, 2 - вычитание, 3 - умножение, 4 - деление, любая другая кнопка - выход: ");
        if(0==scanf("%c",&select)){
            perror("Error!");
            exit(1);
        }
        int a,b,res;
        switch(select){
            case '1':{
                input(&a,&b);
                res=add(a,b);
                break;
            }
            case '2':{
                input(&a,&b);
                res=sub(a,b);
                break;
            }
            case '3':{
                input(&a,&b);
                res=mul(a,b);
                break;
            }
            case '4':{
                input(&a,&b);
                res=divide(a,b);
                break;
            }
            default:{
                return 0;
            }
        }
        printf("Результат: %d\n",res);
        while(getchar()!='\n');
    }
}