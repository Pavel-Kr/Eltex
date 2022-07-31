#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>
struct phone{
    int number;
    char name[20];
    char last[20];
};
void copy(struct phone* a, struct phone* b){
    strcpy(a->last,b->last);
    strcpy(a->name,b->name);
    a->number=b->number;
}
int main(){
    setlocale(LC_ALL,"rus");
    struct phone *phonebook=NULL;
    int size=0;
    while(1){
        char select;
        printf("1 - Добавить номер, 2 - Удалить номер, 3 - Поиск номера, 4 - Показать все номера, Любая другая кнопка - Выход: ");
        scanf("%c",&select);
        switch(select){
            case '1':
            {
                phonebook=realloc(phonebook,(size+1)*sizeof(struct phone));
                if(phonebook==NULL){
                    perror("Out of memory!\n");
                    exit(1);
                }
                printf("Введите фамилию: ");
                scanf("%s",phonebook[size].last);
                printf("Введите имя: ");
                scanf("%s",phonebook[size].name);
                printf("Введите номер: ");
                scanf("%d",&phonebook[size].number);
                size++;
                break;
            }
            case '2':
            {
				int num;
				printf("Какой по порядку номер удалить? ");
				scanf("%d",&num);
                for(num--;num<size-1;num++){
                    copy(&phonebook[num],&phonebook[num+1]);
                }
                phonebook=realloc(phonebook,(size-1)*sizeof(struct phone));
                if(phonebook==NULL){
                    perror("Out of memory!\n");
                    exit(1);
                }
                size--;
				break;
            }
            case '3':
			{
				char last[20];
				printf("Введите фамилию: ");
				scanf("%s",last);
				for(int i=0;i<size;i++){
                    if(strncmp(last,phonebook[i].last,3)==0){
						printf("%d)\t%s %s %d\n",i+1,phonebook[i].last,phonebook[i].name,phonebook[i].number);
					}
				}
				break;
			}
            case '4':
            {
                for(int i=0;i<size;i++){
                    printf("%d)\t%s %s %d\n",i+1,phonebook[i].last,phonebook[i].name,phonebook[i].number);
                }
                break;
            }
            default:
                return 0;
        }
        while (getchar()!='\n');
    }
    free(phonebook);
    return 0;
}