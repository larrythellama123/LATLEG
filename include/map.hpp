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
    void printBucketMap(const auto& bucket_map) {
    if (bucket_map.empty()) {
        std::cout << "[Bucket Map] Empty\n";
        return;
    }

    std::cout << "=== BUCKET MAP CONTENTS ===\n";
    // Loop through each bucket entry (pair of coordinates -> set of enemies)
    for (const auto& [bucket_coords, enemy_set] : bucket_map) {
        std::cout << "Bucket [" << bucket_coords.first << ", " << bucket_coords.second << "]:\n";
        
        if (enemy_set.empty()) {
            std::cout << "  (empty bucket)\n";
            continue;
        }

        // Loop through and print each enemy in this specific bucket
        for (const auto& enemy : enemy_set) {
            std::cout << "  -> Enemy '" << enemy.character << "' "
                      << "Current: (" << static_cast<int>(enemy.x) << ", " << static_cast<int>(enemy.y) << ") "
                      << "Prev: (" << static_cast<int>(enemy.prev_x) << ", " << static_cast<int>(enemy.prev_y) << ")\n";
        }
    }
    std::cout << "===========================\n";
}

    void updatePositionAndBucket(const PlayerPacketInput& PP_input){
        full_map[PP_input.y * 254 + PP_input.x] = PP_input.character;
        full_map[PP_input.prev_y * 254 + PP_input.prev_x] = ' ';

        std::pair<int,int> p1 = {PP_input.x/10,PP_input.y/10};
        std::pair<int,int> p2 = {PP_input.prev_x/10,PP_input.prev_y/10};


        EnemyInfo tmp = {PP_input.prev_x,PP_input.prev_y,0,0,PP_input.character};
        bucket_map[p2].erase(tmp);
        std::cout<<"  p1f "<<p1.first<<" p1s "<<p1.second<<std::endl;
        EnemyInfo tmp2 = {PP_input.x,PP_input.y,PP_input.prev_x, PP_input.prev_y,PP_input.character};
        bucket_map[p1].insert(tmp2);
        // printBucketMap(bucket_map);
        
    }

    
    
    std::vector<EnemyInfo> checkEnemy(const PlayerPacketInput& PP_input){
        std::pair<int,int> p = {PP_input.x/10,PP_input.y/10};
        std::vector<EnemyInfo> enemy_positions;
        for(const auto& dir : directions){
            int tmp1 = p.first + dir[0];
            int tmp2 = p.second + dir[1];   

            if(tmp1 < 0 || tmp1 > 25 || tmp2 < 0 || tmp2 > 25){
                continue;
            }
            std::cout << " tmp 1 " << static_cast<int>(PP_input.x) <<" tmp2  "<< tmp2<<std::endl;
            std::unordered_set<EnemyInfo> set = bucket_map[{tmp1,tmp2}];
            for (const auto& element : set) {
                // if(element.x  ==  PP_input.x && element.y == PP_input.y){
                //     continue;
                // }
                std::cout << "x=" << static_cast<int>(element.x)
                << " y=" << static_cast<int>(element.y)
                << " char=" << element.character << std::endl;
                enemy_positions.emplace_back(element);
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
            PP_output.legal = false;
            PP_output.enemy_positions = checkEnemy(PP_input);
            std::cout<<"DEBUG"<<std::endl;
            return PP_output;
        }
        updatePositionAndBucket( PP_input);
        PP_output.enemy_positions = checkEnemy(PP_input);
        return PP_output;
    }

    private:
        std::vector<std::vector<int>> directions = {{0,0},{1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};
        std::vector<char>full_map;
        std::unordered_map<std::pair<int,int>, std::unordered_set<EnemyInfo>,PairHash> bucket_map;
};
