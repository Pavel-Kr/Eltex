#include <curses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TEXT_LEN 512

void sig_winch(int signo){
    struct winsize size;
    ioctl(fileno(stdout),TIOCGWINSZ,(char*)&size);
    resize_term(size.ws_row,size.ws_col);
}
WINDOW* init_window(){
    struct winsize size;
    ioctl(fileno(stdout),TIOCGWINSZ,(char*)&size);
    return newwin(size.ws_row-1,size.ws_col,0,0);
}
WINDOW* init_control(){
    struct winsize size;
    ioctl(fileno(stdout),TIOCGWINSZ,(char*)&size);
    return newwin(1,size.ws_col,size.ws_row-1,0);
}
void input_text(WINDOW* text_wnd, WINDOW* control_wnd, char text[], int max_len){
    int i=0,j=0;
    int ch;
    int len[30]={0};
    int rows=0;
    while (i<max_len-1)
    {
        ch=wgetch(text_wnd);
        if(ch==KEY_BACKSPACE){
            int x,y;
            getyx(text_wnd,y,x);
            if(i==0){
                if(j==0) continue;
                j--;
                i=len[j];
                wrefresh(text_wnd);
                wmove(text_wnd,y-1,i);
                rows--;
            }
            else{
                mvwaddch(text_wnd,y,x-1,' ');
                wrefresh(text_wnd);
                wmove(text_wnd,y,x-1);
                if(i==len[j]) len[j]--;
                i--;
            }
            continue;
        }
        else if(ch==KEY_LEFT){
            int x,y;
            getyx(text_wnd,y,x);
            if(i==0){
                if(j==0) continue;
                j--;
                i=len[j];
                wmove(text_wnd,y-1,len[j]);
            }
            else{
                wmove(text_wnd,y,x-1);
                i--;
            }
            continue;
        }
        else if(ch==KEY_RIGHT){
            int x,y;
            getyx(text_wnd,y,x);
            if(i==len[j]){
                if(j==rows) continue;
                wmove(text_wnd,y+1,0);
                i=0;
                j++;
            } 
            else{ 
                wmove(text_wnd,y,x+1);
                i++;
            }
            continue;
        }
        else if(ch==KEY_UP){
            if(j==0) continue;
            int x,y;
            getyx(text_wnd,y,x);
            j--;
            if(i>len[j]){
                wmove(text_wnd,y-1,len[j]);
                i=len[j];
            }
            else{
                wmove(text_wnd,y-1,x);
            }
            continue;
        }
        else if(ch==KEY_DOWN){
            if(j==rows) continue;
            int x,y;
            getyx(text_wnd,y,x);
            j++;
            if(i>len[j]){
                wmove(text_wnd,y+1,len[j]);
                i=len[j];
            }
            else{
                wmove(text_wnd,y+1,x);
            }
            continue;
        }
        else if(ch==KEY_F(4)){
            delwin(control_wnd);
            delwin(text_wnd);
            endwin();
            return;
        }
        else if(ch==KEY_F(3)){
            wclear(control_wnd);
            wprintw(control_wnd,"Enter file name: ");
            wrefresh(control_wnd);
            echo();
            char path[101];
            wgetnstr(control_wnd,path,100);
            FILE* file=fopen(path,"w");
            char* ptr=text;
            for(int k=0;k<=rows;k++){
                fwrite(ptr,sizeof(char),len[k],file);
                fwrite("\n",1,1,file);
                ptr+=len[k];
            }
            fclose(file);
            noecho();
            wprintw(control_wnd,"F2 - open file, F3 - save to file, F4 - close");
            wrefresh(control_wnd);
            continue;
        }
        else if(ch==KEY_F(2)){
            wclear(control_wnd);
            wprintw(control_wnd,"Enter file name: ");
            wrefresh(control_wnd);
            echo();
            char path[101];
            wgetnstr(control_wnd,path,100);
            FILE* file=fopen(path,"r");
            if(!file){
                noecho();
                wclear(control_wnd);
                wprintw(control_wnd,"Error: cannot open file, press any key to continue...");
                wrefresh(control_wnd);
                getch();
                wclear(control_wnd);
                wprintw(control_wnd,"F2 - open file, F3 - save to file, F4 - close");
                wrefresh(control_wnd);
                continue;
            }
            j=0,rows=0;
            len[j]=0;
            int c=0;
            int k=0;
            wclear(text_wnd);
            rewind(file);
            while(k<MAX_TEXT_LEN && !feof(file)){
                fread(&c,sizeof(char),1,file);
                if(c=='\n'){
                    j++;
                    rows++;
                    len[j]=0;
                    wechochar(text_wnd,c);
                    continue;
                }
                text[k++]=c;
                len[j]++;
                wechochar(text_wnd,c);
            }
            i=0,j=0;
            text[k]=0;
            fclose(file);
            noecho();
            wmove(text_wnd,0,0);
            wprintw(control_wnd,"F2 - open file, F3 - save to file, F4 - close");
            wrefresh(control_wnd);
            continue;
        }
        else if(ch=='\n'){
            j++;
            i=0;
            rows++;
            wechochar(text_wnd,ch);
            continue;
        }
        if(i==len[j]) len[j]++;
        int pos=0;
        for(int k=0;k<j;k++){
            pos+=len[k];
        }
        pos+=i;
        text[pos++]=ch;
        i++;
        wechochar(text_wnd,ch);
    }
    
}
int main(){
    WINDOW *text_wnd;
    WINDOW *control_wnd;
    char text[MAX_TEXT_LEN+1];
    text[MAX_TEXT_LEN]=0;
    initscr();
    signal(SIGWINCH,sig_winch);
    cbreak();
    noecho();
    curs_set(1);
    start_color();
    refresh();
    text_wnd=init_window();
    control_wnd=init_control();
    keypad(text_wnd,TRUE);
    wprintw(control_wnd,"F2 - open file, F3 - save to file, F4 - close");
    wrefresh(text_wnd);
    wrefresh(control_wnd);
    refresh();
    input_text(text_wnd,control_wnd,text,MAX_TEXT_LEN);
    return 0;
}