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
#include <chrono>

#include <client_manager.h>

using namespace std;

int hello()
{
  printf("hello ");
  return 1;
}

string compose_obj(map_obj obj){
    string out = "|";
    out += "[x]"+to_string(obj.xpos);
    out += "[y]"+to_string(obj.ypos);
    out += "[r]"+to_string(obj.rot);
    out += "[typ]"+obj.type;
    out += "[nam]"+obj.name;
    out += "[msc]"+obj.misc;
    out += "|";
    return out;
}

string compose_map_update(list<map_obj>* gamestate_ptr, map_obj* mypoint = NULL){
    string out = "[mup]";
    list<map_obj>& gamestate = *gamestate_ptr;
    for (std::list<map_obj>::iterator it=gamestate.begin(); it != gamestate.end(); ++it){
        if (&(*it) != mypoint){
            out += compose_obj(*it);
        }
    }
    return out;
}

void manage(map_obj* player_ptr, list<map_obj>* gamestate_ptr, int port, bool* taken_ptr, bool* close_ptr){
    bool& running = *close_ptr;
    list<map_obj>& gamestate = *gamestate_ptr;
    map_obj& player = *player_ptr;

    char buffer[1024] = {0};
    int opt = 1;
    bool connection_good = false;

    struct sockaddr_in in_addr;
    int addr_len = sizeof(in_addr);
    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = INADDR_ANY;
    in_addr.sin_port = htons(port);

    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(sockid, F_SETFL, O_NONBLOCK);

    bind(sockid, (struct sockaddr *)&in_addr, sizeof(in_addr));
    chrono::steady_clock::time_point timeout = chrono::steady_clock::now();
    while(running){
        listen(sockid, 3);
        int connection = accept(sockid, (struct sockaddr *)&in_addr, (socklen_t *)&addr_len);
        if(connection >= 0){
            string message, response;
            read(connection, buffer, 1024);
            message = buffer;
            if (message.substr(0,11) == "[handshake]"){
                response = "[acknowledged]";
                write(connection, response.c_str(), response.length());
                connection_good = true;
                timeout = chrono::steady_clock::now();
                break;
            }else{
                string errmessage = "bad handshake";
                write(connection, errmessage.c_str(), errmessage.length());
            }
        }
        close(connection);
        this_thread::sleep_for(chrono::milliseconds(10));
        if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-timeout).count()>5000){
            break;}
    }
    if (connection_good){
        string upd;
        while(running){
            listen(sockid, 3);
            int connection = accept(sockid, (struct sockaddr *)&in_addr, (socklen_t *)&addr_len);
            if (read(connection, buffer, 1024)!=-1){
                mvprintw(6,10,buffer);
                timeout = chrono::steady_clock::now();
                upd = compose_map_update(gamestate_ptr, player_ptr);
                //write(connection, "test", 4);
                write(connection, upd.c_str(), upd.length());
            }
            if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-timeout).count()>5000){
                cout << "player: " << player.name <<" disconnected - timeout\n";
                break;
            }
            close(connection);
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    for (std::list<map_obj>::iterator it=gamestate.begin(); it != gamestate.end(); ++it){
        if (&(*it) == player_ptr){
            gamestate.erase(it++);
        }
    }
    close(sockid);
    *taken_ptr = false;
}

map_obj::map_obj(string typ, string nam = "", int x = 0, int y = 0, int r = 0, string mis = ""){
    xpos = x;
    ypos = y;
    rot = r;
    type = typ;
    name = nam;
    misc = mis;
}

thread* client_manager(list<map_obj>* gamestate_ptr, string name, int port, bool* taken_ptr, bool* close_ptr){
    list<map_obj>& gamestate = *gamestate_ptr;

    gamestate.push_back(map_obj("player", name));
    map_obj& player = gamestate.back();
    thread* t1 = new thread(manage, &player, gamestate_ptr, port, taken_ptr, close_ptr);
    return t1;
}
