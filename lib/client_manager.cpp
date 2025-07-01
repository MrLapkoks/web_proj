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

int hello()
{
  printf("hello ");
  return 1;
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
    bool& close = *close_ptr;
    list<map_obj>& gamestate = *gamestate_ptr;

    gamestate.push_back(map_obj("player", name, port));
    map_obj& player = gamestate.back();
    thread* t1 = new thread(hello);


    *taken_ptr = false;
    return t1;
}
