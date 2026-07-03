#include "player.hpp"
#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <iterator>

class Renderer{
    public:
        Renderer(){
            std::ifstream file("./text_map.txt", std::ios::binary);
            if(!file){
                endwin(); 
                std::cerr << "file not found\n";
                std::exit(1);
            }

            full_map.reserve(300*300);
            std::string line;
            while (std::getline(file, line)) {
                full_map.insert(full_map.end(), line.begin(), line.end());
            }

           
            // for(int i =0 ; i < 300; i++){
            //     for(int j =0 ; j < 300; j++){
            //         std::cout<<full_map[i*300 + j];
            //     }       
            //     std::cout<<""<<std::endl;
            // }

            chunk.resize(300*300);
            for(int i =0 ; i < 100; i ++){
                for(int j =0 ; j < 100; j++){
                    chunk[i*100+j] = full_map[i*300+j];
                }       
            }

            for (int i = 0; i < 300; i++){
                for (int j = 0; j < 300; j++){
                    move(j, i);
                    addch(full_map[i*300 + j]);
                }
            }
                
            // mvwaddch(pad, i, j, full_map[i*300 + j]);

            // prefresh(pad, 0, 0, 0, 0, 99, 99); 
            refresh();

        }

        void render(const PlayerPacketOutput& PP){
            int top  = std::clamp(PP.y - 50, 0, 300 - 101);
            int left = std::clamp(PP.x - 50, 0, 300 - 101);
            move(PP.y, PP.x);
            addch(PP.character);
            refresh();
            move(PP.y, PP.x);
            addch(full_map[PP.y*300+PP.x]);
        }
        
private:
    int prev_n;
    int prev_s;
    int prev_e;
    int prev_w;
    WINDOW* pad = newpad(300, 300);
    bool is_first_render = true;
    std::vector<char>full_map;
    std::vector<char>chunk;



};