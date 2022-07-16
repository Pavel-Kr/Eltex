#include <stdio.h>
int main(){
    int num=0xaabbccdd;
    char* ptr=(char*)&num;
    for(int i=0;i<4;i++){
        printf("%d byte of number %X = %X\n",i+1,num,*ptr&0xff);
        ptr++;
    }
    ptr=(char*)&num+2;
    *ptr=0xee;
    printf("Changed 3rd byte: %X\n",num);
    return 0;
}