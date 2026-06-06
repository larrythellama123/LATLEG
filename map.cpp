//render only part of the map that can be seen by the player
//if enemy position is within the coords then render the enemy 
//just edit the position change of the player first and 

#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <functional> 

struct EnemyInfo {
    int x;
    int y;
    char character;

    bool operator==(const EnemyInfo& other) const {
        return x == other.x && 
               y == other.y && 
               character == other.character;
    }
};

namespace std {
    template <>
    struct hash<EnemyInfo> {
        std::size_t operator()(const EnemyInfo& e) const {
            std::size_t h1 = std::hash<int>{}(e.x);
            std::size_t h2 = std::hash<int>{}(e.y);
            std::size_t h3 = std::hash<char>{}(e.character);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

struct PlayerPacket {
    uint8_t player_id;
    std::vector<std::pair<std::pair<int,int>, char>> enemy_positions;
    uint8_t x;
    uint8_t y;
    uint8_t prev_x;
    uint8_t prev_y;
    char character;
    PlayerState PS;
};

class Map{
    Map():full_map(302*302){
        std::ifstream file("text_map.txt", std::ios::binary);
        if(!file){
            std::cerr<<"file not found";
        }
        file >> std::noskipws; 
        full_map.assign(std::istream_iterator<char>(file), std::istream_iterator<char>());
        for(int i =0 ; i < 300; i += 10){
            for(int j =0 ; j < 300; j += 10){
                std::pair<int, int> p = {i/10, j/10};
                bucket_map[p] = {};
            }       
        }     
    }

    void updatePositionAndBucket(const PlayerPacket& PP_input){
        full_map[PP_input.y * 300 + PP_input.x] = PP_input.character;
        std::pair<int,int> p1 = {PP_input.x/10,PP_input.y/10};
        std::pair<int,int> p2 = {PP_input.prev_x/10,PP_input.prev_y/10};
        if(p1!=p2){
            EnemyInfo tmp = {PP_input.prev_x,PP_input.prev_y,PP_input.character};
            bucket_map[p2].erase(tmp);
            tmp = {PP_input.x,PP_input.y,PP_input.character};
            bucket_map[p2].insert(tmp);
        }

    }
    
    std::vector<EnemyInfo> checkEnemy( PlayerPacket& PP_output, const PlayerPacket& PP_input){
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
    
    sendUpdate(const PlayerPacket& PP_input){
        PlayerPacket PP_output;
        if(!checkLegal()){
            return PP_output;
        }
        checkEnemy(PP_output, PP_input);
        updatePositionOnMap(PP_output);
        return PP_output;
    }

    private:
        std::vector<std::vector<int>> directions = {{1,1},{-1,1},{1,-1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};
        std::vector<char>full_map;
        std::unordered_map<std::pair<int,int>, std::unordered_set<EnemyInfo>> bucket_map;
}
