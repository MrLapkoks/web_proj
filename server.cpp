#include <iostream>
#include <ncurses.h>
#include <string>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>

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
    char buffer[1024] = {0};
    int opt = 1;

    struct sockaddr_in in_addr;
    int addr_len = sizeof(in_addr);
    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = INADDR_ANY;
    in_addr.sin_port = htons(PORT);

    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(sockid, F_SETFL, O_NONBLOCK);

    bind(sockid, (struct sockaddr *)&in_addr, sizeof(in_addr));
    while(running){
        if (listen(sockid, 3) >= 0) {
            int connection = accept(sockid, (struct sockaddr *)&in_addr, (socklen_t *)&addr_len);
            read(connection, buffer, 1024);
            mvprintw(1,1,buffer);
            const char* hello = "server responded!";
            write(connection, hello, strlen(hello));
            close(connection);
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
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
