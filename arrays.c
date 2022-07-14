#include <stdio.h>
#define N 5
void print_matrix(int arr[][N], int m, int n){
    for(int i=0;i<m;i++){
        for(int j=0;j<n;j++){
            printf("%d ",arr[i][j]);
        }
        printf("\n");
    }
    printf("---------------------------------------------\n");
}
int main(){
    //Задание 1
    int square_matrix[N][N];
    int num=1;
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            square_matrix[i][j]=num;
            num++;
        }
    }
    print_matrix(square_matrix,N,N);
    //Задание 2
    int reverse_arr[N];
    for(int i=0;i<N;i++){
        reverse_arr[i]=i+1;
        printf("%d ",reverse_arr[i]);
    }
    printf("\n");
    int tmp;
    for(int i=0;i<N/2;i++){
        tmp=reverse_arr[i];
        reverse_arr[i]=reverse_arr[N-i-1];
        reverse_arr[N-i-1]=tmp;
    }
    for(int i=0;i<N;i++){
        printf("%d ",reverse_arr[i]);
    }
    printf("\n---------------------------------------------\n");
    //Задание 3
    int triangle_matrix[N][N];
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            triangle_matrix[i][j]=(i+j<N-1) ? 0 : 1;
        }
    }
    print_matrix(triangle_matrix,N,N);
    //Задание 4
    int spiral_matrix[N][N];
    num=1;
    for(int k=0;k<N/2+1;k++){
        int i,j;
        for(i=k,j=k;j<N-k;j++){
            spiral_matrix[i][j]=num++;
        }
        for(i++,j--;i<N-k;i++){
            spiral_matrix[i][j]=num++;
        }
        for(i--,j--;j>=k;j--){
            spiral_matrix[i][j]=num++;
        }
        for(j++,i--;i>k;i--){
            spiral_matrix[i][j]=num++;
        }
    }
    print_matrix(spiral_matrix,N,N);
    return 0;
}