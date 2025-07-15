#include <iostream>
#include <ncurses.h>
#include <string>
#include <list>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <client_manager.h>

using namespace std;

#define MAX_PLAYERS 20

bool running;
string test;

void login(list<map_obj>* gamestate, int PORT){
    bool clients[MAX_PLAYERS];
    thread* client_threads[MAX_PLAYERS];
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
        listen(sockid, 3);
        int connection = accept(sockid, (struct sockaddr *)&in_addr, (socklen_t *)&addr_len);
        if(connection >= 0){
            int id;
            string message, response;
            for (id = 0; id<MAX_PLAYERS; id++){
                if(clients[id] == false) break;
            }
            read(connection, buffer, 1024);
            mvprintw(1, 1, buffer);
            mvprintw(2, 2, to_string(running).c_str());
            message = buffer;
            if (message.substr(0,15) == "[login request]"){
                string name = message.substr(15, message.find("|")-15);
                response = name+"|"+to_string(PORT+1+id);
                //test +="2";
                if(id < 20){
                    clients[id] = true;
                    if(client_threads[id]){
                        client_threads[id]->join();
                        client_threads[id] = NULL;
                    }
                    client_threads[id] = client_manager(gamestate, name, PORT+1+id, &clients[id], &running);
                    //pass:
                    //map pointer
                    //clients[id] pointer
                    //player pointer
                    //port
                    //pointer to "close"
                    write(connection, response.c_str(), response.length());
                }
                else{
                    string errmessage = "login denied - server full";
                    write(connection, errmessage.c_str(), errmessage.length());
                }
            }
            else{
                string errmessage = "login denied - bad request";
                write(connection, errmessage.c_str(), errmessage.length());
            }
        }
        close(connection);
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    close(sockid);
    for (int n = 0; n < MAX_PLAYERS; n++){
        if (clients[n] == true){
            client_threads[n]->join();
        }
    }
}

int main() {
    list<map_obj> gamestate;
    running = true;
    char ch;
    WINDOW* win = initscr();
    noecho();
    nodelay(win, true);
    thread join_th(login, &gamestate, 1234);
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
        mvprintw(0,0,to_string(gamestate.size()).c_str());
    }
    join_th.join();
    mvprintw(5,5, "AAAAAA");
    nodelay(win, false);
    echo();
    endwin();
    //cout << test;
    return 0;
}
