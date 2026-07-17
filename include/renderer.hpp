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
            refresh();

        }

        void render(const PlayerPacketOutput& PP){

            for (int i = std::max(PP.x - 10, 0); i < std::min(PP.x+10, 254); i++){
                for (int j = std::max(PP.y - 10, 0); j < std::min(PP.y+10, 254); j++){
                    move(j, i);
                    addch(full_map[j*254 + i]);
                }
            }

            if(PP.active_player){
                move(PP.y, PP.x);
                addch(PP.character);
                refresh();
            }
            render_only_enemies(PP);
        }
        
        void render_only_enemies(const PlayerPacketOutput& PP){
            //fix later: renders 10 by 10 square around the player starting pos, but if enemies are over tha map,
            //their prev positions remain
            
            for (int i = std::max(PP.x - 10, 0); i < std::min(PP.x+10, 254); i++){
                for (int j = std::max(PP.y - 10, 0); j < std::min(PP.y+10, 254); j++){
                    move(j, i);
                    addch(full_map[j*254 + i]);
                }
            }

            for(const auto& enemy_pos: PP.enemy_positions){
                move(enemy_pos.y, enemy_pos.x);
                addch(enemy_pos.character);    
            } 

            refresh();
        }
        
private:
    WINDOW* pad = newpad(254, 254);
    std::vector<char>full_map;
    std::vector<char>chunk;



};