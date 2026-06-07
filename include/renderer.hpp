#include "player.hpp"
#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <iterator>

class Renderer{
    Renderer():{
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
                chunk[i*100+j] = full_map[i*300+j]
            }       
        }     
    }

    void render(const PlayerPacketOutput& PP){
        if(is_first_render){
            for(int i =0 ; i < 100; i ++){
                for(int j =0 ; j < 100; j++){
                    mvaddch(y, x, chunk[i+100 + j]);
                }       
            }
        }
        else{
            
        }
        
        // Flush the virtual buffer to the actual visible terminal screen
        refresh();            

        // Wait for user input
        getch();              
    }

    void close(){
        endwin();             
    }
private:
    bool is_first_render = true;
    std::vector<char>full_map;
    std::vector<char>chunk;



}