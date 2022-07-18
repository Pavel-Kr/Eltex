#include <stdio.h>
#include <locale.h>
#include <string.h>
int global_time=0;
struct phone{
    int number;
    char name[20];
    char last[20];
    int time;
};
int main(){
    setlocale(LC_ALL,"rus");
    struct phone phonebook[10];
    int indexes[10]={0,1,2,3,4,5,6,7,8,9};
    while(1){
        char select;
        printf("1 - Добавить номер, 2 - Удалить номер, 3 - Поиск номера, 4 - Показать все номера, Любая другая кнопка - Выход: ");
        scanf("%c",&select);
        switch(select){
            case '1':
            {
                int index=-1;
                int min_time=9999999;
                for(int i=0;i<10;i++){
                    if(indexes[i]!=-1){
                        index=indexes[i];
                        indexes[i]=-1;
                        break;
                    }
                    if(phonebook[i].time<min_time){
                        min_time=phonebook[i].time;
                        index=i;
                    }
                }
                printf("Введите фамилию: ");
                scanf("%s",phonebook[index].last);
                printf("Введите имя: ");
                scanf("%s",phonebook[index].name);
                printf("Введите номер: ");
                scanf("%d",&phonebook[index].number);
                phonebook[index].time=global_time++;
                break;
            }
            case '2':
            {
				int num;
				printf("Какой по порядку номер удалить? ");
				scanf("%d",&num);
				num--;
				if(num<0||num>=10) break;
				indexes[num]=num;
				break;
            }
            case '3':
			{
				char last[20];
				printf("Введите фамилию: ");
				scanf("%s",last);
				for(int i=0;i<10;i++){
					if(indexes[i]==-1){
						if(strncmp(last,phonebook[i].last,3)==0){
							printf("%d)\t%s %s %d\n",i+1,phonebook[i].last,phonebook[i].name,phonebook[i].number);
						}
					}
				}
				break;
			}
            case '4':
            {
                for(int i=0;i<10;i++){
                    if(indexes[i]==-1) printf("%d)\t%s %s %d\n",i+1,phonebook[i].last,phonebook[i].name,phonebook[i].number);
                }
                break;
            }
            default:
                return 0;
        }
        while (getchar()!='\n');
    }
    return 0;
}