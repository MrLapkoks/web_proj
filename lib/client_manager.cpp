#include <iostream>
#include <ncurses.h>
#include <string>
#include <list>
#include <chrono>
#include <thread>
#include <mutex>
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

string add_money(int amnt, const string misc_f){
    string tag = "[MON]";
    string z = "0";
    int pos = misc_f.find(tag);
    if (pos == -1){
        return misc_f + tag + to_string(amnt)+"|";
    }
    int end = misc_f.find("|", pos+tag.length());
    if (end == -1){
        return misc_f + tag + to_string(amnt)+"|";
    }
    int money = stoi(z+misc_f.substr(pos+tag.length()));
    money = money+amnt;
    string o = misc_f.substr(0,pos+tag.length());
    o += to_string(money);
    o += misc_f.substr(end);
    return o;
}

string get_time_str(){
    chrono::system_clock::time_point timepoint = chrono::system_clock::now();
    chrono::system_clock::duration td;
    int hour = chrono::duration_cast<chrono::hours>(timepoint.time_since_epoch()).count();
    td = (chrono::hours)hour;
    int min = chrono::duration_cast<chrono::minutes>(timepoint.time_since_epoch()-td).count();
    td += (chrono::minutes)min;
    int sec = chrono::duration_cast<chrono::seconds>(timepoint.time_since_epoch()-td).count();
    td += (chrono::seconds)sec;
    int millis = chrono::duration_cast<chrono::milliseconds>(timepoint.time_since_epoch()-td).count();
    return to_string(hour) + ":" + to_string(min) + ":" + to_string(sec) + ":" + to_string(millis);
}

int get_time_diff (string past){
    int point = 0;
    int h = stoi(past);
    point = past.find(":")+1;
    int m = stoi(past.substr(point));
    point = past.find(":", point)+1;
    int s = stoi(past.substr(point));
    point = past.find(":", point)+1;
    int ms = stoi(past.substr(point));
    chrono::milliseconds past_p = (chrono::hours)h+(chrono::minutes)m+(chrono::seconds)s+(chrono::milliseconds)ms;
    chrono::system_clock::time_point timenow = chrono::system_clock::now();


    return chrono::duration_cast<chrono::milliseconds>(timenow.time_since_epoch()-past_p).count();
}

string shoot(string x, string y, string r, const string misc_f){
    return misc_f + "|[SHOT][X]" + x + "[Y]" + y + "[R]" + r + "[T]" + get_time_str()+"|";
}

string died(string player, const string misc_f){
    return misc_f + "|[KILL][P]" + player + "[T]" + get_time_str()+"|";
}

string clear_outdated(const string misc_f){
    //get_time_diff(time)
    string o = misc_f;
    int i = 0;
    while (o.find("|[SHOT]", i) != -1){
        i = o.find("|[SHOT]", i)+1;
        int t_p = o.find("[T]", i);
        int end_p = o.find("|", t_p);
        if(get_time_diff(o.substr(t_p+3, end_p-t_p-3))> 2000){
            i--;
            o = o.substr(0,i)+o.substr(end_p);
        }
    }
    i = 0;
    while (o.find("|[KILL]", i) != -1){
        i = o.find("|[KILL]", i)+1;
        int t_p = o.find("[T]", i);
        int end_p = o.find("|", t_p);
        if(get_time_diff(o.substr(t_p+3, end_p-t_p-3))> 2000){
            i--;
            o = o.substr(0,i)+o.substr(end_p);
        }
    }

    return o;

}

string compose_obj(map_obj obj){
    string out = "|";
    out += "[X]"+to_string(obj.xpos);
    out += "[Y]"+to_string(obj.ypos);
    out += "[R]"+to_string(obj.rot);
    out += "[TYP]"+obj.type;
    out += "[NAM]"+obj.name;
    out += "[MSC]"+obj.misc;
    out += "|";
    return out;
}

string compose_map_update(list<map_obj>* gamestate_ptr, map_obj* mypoint = NULL){
    string out = "[MUP]";
    list<map_obj>& gamestate = *gamestate_ptr;
    for (std::list<map_obj>::iterator it=gamestate.begin(); it != gamestate.end(); ++it){
        if (&(*it) != mypoint){
            out += compose_obj(*it);
        }
    }
    out += "[TIME]" + get_time_str();
    return out;
}

int try_action(list<map_obj>* gamestate_ptr, int x, int y, string type){
    list<map_obj>& gamestate = *gamestate_ptr;
    for (std::list<map_obj>::iterator it=gamestate.begin(); it != gamestate.end(); ++it){
        if (it->xpos == x && it->ypos == y){
            if (it->name == type){
                return it->advance();
            } else{
                return 0;
            }
        }
    }
    if (type == "none"){
        gamestate_ptr->push_back(map_obj("field", "none", x, y));
        return gamestate.back().advance();
    }
    return 0;
}

void manage(map_obj* player_ptr, list<map_obj>* gamestate_ptr, int port, bool* taken_ptr, bool* close_ptr, int id, mutex* mtx_ptr){
    mutex& mtx = *mtx_ptr;
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
                mvprintw(3*id+2,25,response.c_str());
                connection_good = true;
                timeout = chrono::steady_clock::now();
                break;
            }else{
                string errmessage = "bad handshake";
                mvprintw(3*id+2,25,errmessage.c_str());
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
                string rec = buffer;
                if (rec.substr(0,6) == "|[PUP]"){
                    int x_p = rec.find("[X]");
                    int y_p = rec.find("[Y]");
                    int r_p = rec.find("[R]");
                    if (x_p != -1 && y_p != -1 && r_p != -1){
                        player.xpos = stoi(rec.substr(x_p+3));
                        player.ypos = stoi(rec.substr(y_p+3));
                        player.rot = stoi(rec.substr(r_p+3));
                    }
                }else if (rec.substr(0,6) == "|[ACT]"){
                    int x_p = rec.find("[X]");
                    int y_p = rec.find("[Y]");
                    int t_p = rec.find("[T]");
                    int end_p = rec.find("|", 1);
                    if (x_p != -1 && y_p != -1 && t_p != -1 && end_p != -1){
                        int money;
                        mtx.lock();
                            money = try_action(gamestate_ptr, stoi(rec.substr(x_p+3)), stoi(rec.substr(y_p+3)), rec.substr(t_p+3, end_p-t_p-3));
                        mtx.unlock();
                        if (money){
                            player.misc = add_money(money, player.misc);
                        }
                    }
                }else if (rec.substr(0,6) == "|[SHT]"){
                    int x_p = rec.find("[X]");
                    int y_p = rec.find("[Y]");
                    int r_p = rec.find("[R]");
                    int end_p = rec.find("|", 1);
                    if (x_p != -1 && y_p != -1 && r_p != -1 && end_p != -1){
                        player.misc = shoot(rec.substr(x_p+3, y_p-x_p-3), rec.substr(y_p+3, r_p-y_p-3), rec.substr(r_p+3, end_p-r_p-3), player.misc);
                    }
                }else if (rec.substr(0,6) == "|[DED]"){
                    int pl_p = rec.find("[P]");
                    int end_p = rec.find("|", 1);
                    if (pl_p != -1 && end_p != -1){
                        player.misc = died(rec.substr(pl_p+3, end_p-pl_p-3), player.misc);
                    }
                }else if (rec.substr(0,6) == "|[COL]"){
                    string z = "0";
                    int amnt = stoi(z+rec.substr(6));
                    player.misc = add_money(amnt, player.misc);
                }
                string clear_out = clear_outdated(player.misc);
                if(clear_out != player.misc){
                    player.misc = clear_out;
                }
                mvprintw(3*id,25,buffer);

                timeout = chrono::steady_clock::now();
                upd = compose_map_update(gamestate_ptr, player_ptr);
                //write(connection, "test", 4);
                write(connection, upd.c_str(), upd.length());
                mvprintw(3*id+1,25,upd.c_str());
            }
            if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-timeout).count()>5000){
                string dc_msg = "player: " + player.name +" disconnected - timeout";
                mvprintw(3*id+2,25,dc_msg.c_str());
                break;
            }
            close(connection);
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    for (std::list<map_obj>::iterator it=gamestate.begin(); it != gamestate.end(); ++it){
        if (&(*it) == player_ptr){
            mtx.lock();
            gamestate.erase(it++);
            mtx.unlock();
        }
    }
    close(sockid);
    *taken_ptr = false;
}

//map_obj::map_obj(string typ, string nam = "", int x = 0, int y = 0, int r = 0, string mis = ""){
map_obj::map_obj(string typ, string nam, int x, int y, int r, string mis){
    xpos = x;
    ypos = y;
    rot = r;
    type = typ;
    name = nam;
    misc = mis;
}

int map_obj::advance(){
    // none -> empty -> sown -> watered -> none
    if (name == "none"){
        name == "empty";
        misc = get_time_str();
        return 10;
    }
    if (name == "empty"){
        name == "sown";
        misc = get_time_str();
        return 20;
    }
    if (name == "sown"){
        name == "watered";
        misc = get_time_str();
        return 40;
    }
    if (name == "watered"){
        name == "none";
        misc = get_time_str();
        return 80;
    }
    return 0;
}

thread* client_manager(list<map_obj>* gamestate_ptr, string name, int port, bool* taken_ptr, bool* close_ptr, int id, mutex* mtx_ptr){
    mutex& mtx = *mtx_ptr;
    list<map_obj>& gamestate = *gamestate_ptr;
    mtx.lock();
    gamestate.push_back(map_obj("player", name+"#"+to_string(id)));
    map_obj& player = gamestate.back();
    mtx.unlock();
    thread* t1 = new thread(manage, &player, gamestate_ptr, port, taken_ptr, close_ptr, id, mtx_ptr);
    return t1;
}
