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

struct map_obj{
  int xpos;
  int ypos;
  int rot;
  string type;
  string name;
  string misc;
};

int hello()
{
  printf("hello ");
  return 1;
}

thread client_manager(list<map_obj>* gamestate_ptr, map_obj* player_ptr, int PORT, bool* taken_ptr, bool* close_ptr){
    bool close = &close_ptr;
    thread t1(hello);



    *taken_ptr = false;
    return t1;
}
                    //clients[id] pointer
