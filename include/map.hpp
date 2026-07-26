//render only part of the map that can be seen by the player
//if enemy position is within the coords then render the enemy 
//just edit the position change of the player first and 

#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <functional> 
#include <chrono>
#include "player.hpp"



class Map{
    public:
    Map(){
        std::ifstream file("text_map.txt", std::ios::binary);
        if(!file){
            std::cerr<<"file not found";
        }
        full_map.reserve(254*254);
        std::string line;
        while (std::getline(file, line)) {
            full_map.insert(full_map.end(), line.begin(), line.end());
        }
        for(int i =0 ; i < 254; i += 10){
            for(int j =0 ; j < 254; j += 10){
                std::pair<int, int> p = {i/10, j/10};
                bucket_map[p] = {};
            }       
        }     
    }
    


    void updatePositionAndBucket(const PlayerPacketInput& PP_input, uint8_t player_id, uint8_t current_it){
        if(player_id == current_it){
            enemy_map[player_id] = {player_id, PP_input.x,PP_input.y,PP_input.prev_x, PP_input.prev_y, COLOR::RED, PP_input.character};
        }
        else{
            enemy_map[player_id] = {player_id, PP_input.x,PP_input.y,PP_input.prev_x, PP_input.prev_y, COLOR::WHITE, PP_input.character};
        }
    }

    
    std::vector<EnemyInfo> checkEnemy(const PlayerPacketInput& PP_input, PlayerPacketOutput& PP_output,  uint8_t& current_it, bool& run_enemy_touch){
        std::vector<EnemyInfo> enemy_positions;
        
        for(auto& [player_id, player_info]  : enemy_map){
            if(PP_output.player_id == player_id){
                enemy_positions.emplace_back(player_info);
                continue;
            }
            if(run_enemy_touch  && player_id == current_it && player_info.x == PP_output.x && player_info.y == PP_output.y ){
                run_enemy_touch = false;
                player_info.color = COLOR::WHITE;
                current_it = PP_output.player_id;
            }

            else if(run_enemy_touch  && PP_output.player_id == current_it && player_info.x == PP_output.x && player_info.y == PP_output.y){
                run_enemy_touch = false;
                player_info.color = COLOR::RED;
                current_it = player_id;
            }
            enemy_positions.emplace_back(player_info);
        }
        return enemy_positions;
    }

    bool checkLegal(const PlayerPacketInput& PP_input){
        if(PP_input.x < 0 || PP_input.x > 254){
            return false;    
        }
        if(PP_input.y < 0 || PP_input.y > 254){
            return false;    
        }
        if(full_map[PP_input.y*254 + PP_input.x] == '*'){
            return false;    
        } 
        return true;
    }

    
    PlayerPacketOutput sendUpdate(const PlayerPacketInput& PP_input, uint8_t player_id, uint8_t& current_it, bool& run_enemy_touch){
        PlayerPacketOutput PP_output;
        PP_output.player_id = player_id;
        PP_output.seq_num = PP_input.seq_num;
        PP_output.x = PP_input.x;
        PP_output.y = PP_input.y;
        PP_output.prev_x = PP_input.prev_x;
        PP_output.prev_y = PP_input.prev_y;
        PP_output.character = PP_input.character;

        if(!checkLegal(PP_input)){
            PP_output.x = PP_output.prev_x;
            PP_output.y = PP_output.prev_y;
            PP_output.legal = false;
            PP_output.enemy_positions = checkEnemy(PP_input,PP_output, current_it, run_enemy_touch);
            std::cout<<"DEBUG"<<std::endl;
            return PP_output;
        }
        updatePositionAndBucket( PP_input, player_id, current_it);
        PP_output.enemy_positions = checkEnemy(PP_input, PP_output, current_it, run_enemy_touch);
        updatePositionAndBucket( PP_input, player_id, current_it);
        return PP_output;
    }

    private:
        std::vector<std::vector<int>> directions = {{0,0},{1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};
        std::vector<char>full_map;
        std::unordered_map<std::pair<int,int>, std::unordered_set<EnemyInfo>,PairHash> bucket_map;
        std::unordered_map<uint8_t,EnemyInfo> enemy_map;
};
