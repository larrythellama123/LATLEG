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

    void updatePositionAndBucket(const PlayerPacketInput& PP_input){
        full_map[PP_input.y * 254 + PP_input.x] = PP_input.character;
        full_map[PP_input.prev_y * 254 + PP_input.prev_x] = ' ';

        std::pair<int,int> p1 = {PP_input.x/10,PP_input.y/10};
        std::pair<int,int> p2 = {PP_input.prev_x/10,PP_input.prev_y/10};
        if(p1!=p2){
            EnemyInfo tmp = {PP_input.prev_x,PP_input.prev_y,PP_input.character};
            bucket_map[p2].erase(tmp);
            tmp = {PP_input.x,PP_input.y,PP_input.character};
            bucket_map[p2].insert(tmp);
        }
    }
    
    std::vector<EnemyInfo> checkEnemy(const PlayerPacketInput& PP_input){
        std::pair<int,int> p = {PP_input.x/10,PP_input.y/10};
        std::vector<EnemyInfo> enemy_positions;
        for(std::vector<int> dir : directions){
            std::unordered_set<EnemyInfo> set = bucket_map[{p.first+dir[0],p.second+dir[1]}];
            for (const auto& element : set) {
                enemy_positions.push_back(element);
            }
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
    
    PlayerPacketOutput sendUpdate(const PlayerPacketInput& PP_input){
        PlayerPacketOutput PP_output;
        PP_output.x = PP_input.x;
        PP_output.y = PP_input.y;
        PP_output.prev_x = PP_input.prev_x;
        PP_output.prev_y = PP_input.prev_y;
        PP_output.character = PP_input.character;

        if(!checkLegal(PP_input)){
            PP_output.x = PP_output.prev_x;
            PP_output.y = PP_output.prev_y;
            std::cout<<"DEBUG"<<std::endl;
            return PP_output;
        }
        PP_output.enemy_positions = checkEnemy(PP_input);
        updatePositionAndBucket( PP_input);
        // std::cout<<static_cast<int>(PP_output.x)<<" " << static_cast<int>(PP_output.y)<<std::endl;
        return PP_output;
    }

    private:
        std::vector<std::vector<int>> directions = {{1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};
        std::vector<char>full_map;
        std::unordered_map<std::pair<int,int>, std::unordered_set<EnemyInfo>,PairHash> bucket_map;
};
