#include <iostream>
#include <ncurses.h>
#include <string>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>

using namespace std;

struct {
  int xpos;
  int ypos;
  float rot;
  string type;
  string name;
  string misc;
} map_obj;

bool running;
string test;

void login(int PORT){
    // https://www.tutorialspoint.com/cplusplus/cpp_socket_programming.htm
    int opt = 1;
    int a;
    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    //int new_socket;
    struct sockaddr_in in_addr;
    int addr_len = sizeof(in_addr);
    char buffer[1024] = {0};

    setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = INADDR_ANY;
    in_addr.sin_port = htons(PORT);
    bind(sockid, (struct sockaddr *)&in_addr, sizeof(in_addr));
    //while(running){
        if (listen(sockid, 3) >= 0) {
            a = accept(sockid, (struct sockaddr *)&in_addr, (socklen_t *)&addr_len);
        }
        if (a>0){
            mvprintw(1,1,"listening");
        }
        read(a, buffer, 1024);
        mvprintw(2,2,buffer);
        close(a);
        this_thread::sleep_for(chrono::milliseconds(10));
    //}
    close(sockid);
    test = buffer;
}

int main() {
    running = true;
    char ch;
    WINDOW* win = initscr();
    noecho();
    nodelay(win, true);
    thread join(login, 1234);
    while(true){
        this_thread::sleep_for(chrono::milliseconds(10));
        ch = getch();
        if (ch == 'q'){
            running = false;
            break;
        }else if (ch != -1){
            mvaddch(5,5,'[');
            addch(ch);
            addch(']');
        }
    }
    join.join();
    mvprintw(5,5, "AAAAAA");
    nodelay(win, false);
    echo();
    endwin();
    cout << test;
    return 0;
}
