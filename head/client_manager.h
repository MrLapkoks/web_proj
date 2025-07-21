
using namespace std;

class map_obj{
  public:
    map_obj(string typ, string nam = "", int x = 0, int y = 0, int r = 0, string mis = "");
    //map_obj(string type, string name, int x, int y, int rot, string misc);
    int advance();
    int xpos;
    int ypos;
    int rot;
    string type;
    string name;
    string misc;
};

int hello();
std::thread* client_manager(std::list<map_obj>* gamestate_ptr, string name, int PORT, bool* taken_ptr, bool* close_ptr, int id, mutex* mtx_ptr);
