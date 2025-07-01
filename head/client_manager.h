struct map_obj;

int hello();
std::thread client_manager(std::list<map_obj>* gamestate_ptr, map_obj* player_ptr, int PORT, bool* taken_ptr, bool* close_ptr);
