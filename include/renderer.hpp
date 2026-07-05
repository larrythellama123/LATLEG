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

            full_map.reserve(254*254);
            std::string line;
            while (std::getline(file, line)) {
                full_map.insert(full_map.end(), line.begin(), line.end());
            }

           
            // for(int i =0 ; i < 254; i++){
            //     for(int j =0 ; j < 254; j++){
            //         std::cout<<full_map[i*254 + j];
            //     }       
            //     std::cout<<""<<std::endl;
            // }

            chunk.resize(254*254);
            for(int i =0 ; i < 100; i ++){
                for(int j =0 ; j < 100; j++){
                    chunk[i*100+j] = full_map[i*254+j];
                }       
            }

            for (int i = 0; i < 254; i++){
                for (int j = 0; j < 254; j++){
                    move(j, i);
                    addch(full_map[j*254 + i]);
                }
            }
                
            // mvwaddch(pad, i, j, full_map[i*254 + j]);

            // prefresh(pad, 0, 0, 0, 0, 99, 99); 
            refresh();

        }

        void render(const PlayerPacketOutput& PP){
            // int top  = std::clamp(PP.y - 50, 0, 254 - 101);
            // int left = std::clamp(PP.x - 50, 0, 254 - 101);
            move(PP.prev_y, PP.prev_x);
            addch(full_map[PP.prev_y*254+PP.prev_x]);
            move(PP.y, PP.x);
            addch(PP.character);
            refresh();
            
        }
        
private:
    int prev_n;
    int prev_s;
    int prev_e;
    int prev_w;
    WINDOW* pad = newpad(254, 254);
    bool is_first_render = true;
    std::vector<char>full_map;
    std::vector<char>chunk;



};