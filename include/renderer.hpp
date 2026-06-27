#include "player.hpp"
#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <iterator>

class Renderer{
    Renderer(){
        std::ifstream file("text_map.txt", std::ios::binary);
        if(!file){
            std::cerr<<"file not found";
        }
        full_map.reserve(300*300);
        std::string line;
        while (std::getline(file, line)) {
            full_map.insert(full_map.end(), line.begin(), line.end());
        }
        chunk.reserve(300*300);
        for(int i =0 ; i < 100; i ++){
            for(int j =0 ; j < 100; j++){
                chunk[i*100+j] = full_map[i*300+j];
            }       
        }
        WINDOW* pad = newpad(300, 300);
        for (int i = 0; i < 300; i++)
            for (int j = 0; j < 300; j++)
                mvwaddch(pad, i, j, full_map[i*300 + j]);
    }

    void render(const PlayerPacketOutput& PP){
        int top  = std::clamp(PP.y - 50, 0, 300 - 101);
        int left = std::clamp(PP.x - 50, 0, 300 - 101);
        mvwaddch(pad, PP.y, PP.x, PP.character);          
        prefresh(pad, top, left, 0, 0, 100, 100);         
        mvwaddch(pad, PP.y, PP.x, full_map[PP.y*300+PP.x]);     
    }

    void close(){
        endwin();             
    }
private:
    int prev_n;
    int prev_s;
    int prev_e;
    int prev_w;
    
    bool is_first_render = true;
    std::vector<char>full_map;
    std::vector<char>chunk;



}