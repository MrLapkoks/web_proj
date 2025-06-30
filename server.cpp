#include <iostream>
#include <ncurses.h>
#include <string>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

struct {
  int xpos;
  int ypos;
  float rot;
  string type;
  string name;
  string misc;
} map_obj;

bool close;

void login(int PORT){
    int opt = 1;
    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in in_addr;

    setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = INADDR_ANY;
    in_addr.sin_port = htons(PORT);
    bind(sockid, (struct sockaddr *)&in_addr, sizeof(in_addr));


}

int main() {
    close = false;
    char ch;
    WINDOW* win = initscr();
    noecho();
    nodelay(win, true);
    while(true){
        this_thread::sleep_for(chrono::milliseconds(10));
        ch = getch();
        if (ch == 'q'){
            break;
        }else if (ch != -1){
            mvaddch(5,5,'[');
            addch(ch);
            addch(']');
        }
    }
    mvprintw(5,5, "AAAAAA");
    nodelay(win, false);
    echo();
    endwin();
    return 0;
}
