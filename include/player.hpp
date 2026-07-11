#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <ncurses.h>
#include <random> 
#include <stdint.h>

enum class PlayerState:uint8_t {
    Alive,    
    Dead,
};

struct EnemyInfo {
    uint8_t x;
    uint8_t y;
    uint8_t prev_x;
    uint8_t prev_y;
    char character;

    bool operator==(const EnemyInfo& other) const {
        return x == other.x && 
               y == other.y && 
               character == other.character;
    }
};

struct PairHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        // Combine the two hashes using a bitwise XOR and a shift to avoid collisions
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
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

struct PlayerPacketInput {
    public:
    uint8_t prev_x;
    uint8_t prev_y;
    uint8_t x;
    uint8_t y;
    uint8_t seq_num;
    char character;
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer;
        buffer.push_back(x);
        buffer.push_back(y);
        buffer.push_back(prev_x);
        buffer.push_back(prev_y);
        buffer.push_back(seq_num);
        buffer.push_back(static_cast<uint8_t>(character));
        return buffer;
    }

    static PlayerPacketInput deserialize(const std::vector<uint8_t>& buffer){
        PlayerPacketInput PP;
        size_t offset = 0;

        PP.x = buffer[offset++];
        PP.y = buffer[offset++];
        PP.prev_x = buffer[offset++];
        PP.prev_y = buffer[offset++];
        PP.seq_num = buffer[offset++];
        PP.character = static_cast<char>(buffer[offset++]);
        return PP;
    }
};

struct PlayerPacketOutput {
public:
    uint8_t player_id;
    bool legal =  true;
    bool active_player = true;
    std::vector<EnemyInfo> enemy_positions;
    uint8_t x;
    uint8_t y;
    uint8_t prev_x;
    uint8_t prev_y;
    uint8_t seq_num;
    char character;
    PlayerState PS;

    std::vector<uint8_t> serialize(){
        std::vector<uint8_t> buffer;
        buffer.push_back(player_id);
        buffer.push_back(static_cast<uint8_t>(active_player));
        buffer.push_back(x);
        buffer.push_back(y);
        buffer.push_back(prev_x);
        buffer.push_back(prev_y);
        buffer.push_back(seq_num);
        buffer.push_back(static_cast<uint8_t>(character));
        buffer.push_back(static_cast<uint8_t>(PS));
        uint8_t enemy_count = static_cast<uint8_t>(enemy_positions.size());
        buffer.push_back(enemy_count);
        for (const auto& enemy : enemy_positions) {
            buffer.push_back(static_cast<uint8_t>(enemy.character));
            buffer.push_back(enemy.x);
            buffer.push_back(enemy.y);
            if(!active_player){
                std::cout<<static_cast<int>(enemy.y) <<" enemy serilaization "<< static_cast<int>(enemy.x)<<std::endl;
            }

            buffer.push_back(enemy.prev_x);
            buffer.push_back(enemy.prev_y);
        }
        return buffer;
    }

    static PlayerPacketOutput deserialize(const std::vector<uint8_t>& buffer){
        PlayerPacketOutput PP;
        size_t offset = 0;
        PP.player_id = buffer[offset++];
        PP.active_player = static_cast<bool>(buffer[offset++]);
        PP.x = buffer[offset++];
        PP.y = buffer[offset++];
        PP.prev_x = buffer[offset++];
        PP.prev_y = buffer[offset++];
        PP.seq_num = buffer[offset++];
        PP.character = static_cast<char>(buffer[offset++]);
        PP.PS = static_cast<PlayerState>(buffer[offset++]);
        uint8_t enemy_count = buffer[offset++];
        for (uint8_t i = 0; i < enemy_count; ++i) {
            EnemyInfo enemy;
            enemy.character = static_cast<char>(buffer[offset++]);
            enemy.x  = buffer[offset++];
            enemy.y  = buffer[offset++];
            enemy.prev_x  = buffer[offset++];
            enemy.prev_y  = buffer[offset++];
            PP.enemy_positions.push_back(enemy);
        }
        return PP;
    }
};

class Player {
public:
    // Default constructor zeroes out the array automatically
    Player(){
        initscr();             
        noecho();              
        nodelay(stdscr, TRUE); 
        cbreak();
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

    }

    ~Player(){
        endwin(); 
    }

    uint8_t get_seq_num(){
        if(seq_num == 255)seq_num = 0;
        return seq_num;
    }

    void add_seq_num(){
        seq_num++;
    }

    bool processInput() {
        bool change  = false;
        // Store the previous position
        
        // Read a single character from the terminal input buffer
        int ch = getch(); 

        // Process the key press
        switch (ch) {
            case 'w':
            case 'W':
            prev_x = x;
            prev_y = y;
            change  = true;
            y =safe_sub(y,1);
                break;
            case 's':
            case 'S':
            prev_x = x;
            prev_y = y;
            change  = true;
            y =safe_add(y,1);
                break;
            case 'a':
            case 'A':
            prev_x = x;
            prev_y = y;
            change  = true;
            x =safe_sub(x,1);
                break;
            case 'd':
            case 'D':
            prev_x = x;
            prev_y = y;
            change  = true;
            x =safe_add(x,1);
                break;
            default:
                // No movement or unhandled key
                break;
        }
        return change;
    }
    

    bool AI_move(){
        bool change = false;
        
        // 1. Obtain a random seed from the hardware
        std::random_device rd; 
        
        // 2. Initialize the standard mersenne_twister_engine with the seed
        std::mt19937 gen(rd()); 
        
        // 3. Define the distribution range [inclusive, inclusive]
        std::uniform_int_distribution<int> distrib(1, 4); 

        // 4. Generate the random number
        int randomNum = distrib(gen); 
        uint8_t tmp;
        switch (randomNum) {
            case 1:
            tmp = y; 
            y = safe_sub(y,1);
            if(!checkLegal(x,y)){
                y = tmp;
            }
            else{
                change  = true;
                prev_x = x;
                prev_y = tmp;
            }
            break;
            case 2:
            tmp = y; 
            y = safe_add(y,1);
            if(!checkLegal(x,y)){
                y = tmp;
            }
            else{
                change  = true;
                prev_x = x;
                prev_y = tmp;
            }
                break;
            case 3:
            tmp = x;
            x = safe_sub(x,1);
            if(!checkLegal(x,y)){
                x = tmp;
            }
            else{
                change  = true;
                prev_x = tmp;
                prev_y = y;
            }
                break;
            case 4:
            tmp = x;
            x = safe_add(x,1);
            if(!checkLegal(x,y)){
                x = tmp;
            }
            else{
                change  = true;
                prev_x = tmp;
                prev_y = y  ;
            }
                break;
            default:
                break;
        }
        return change;
    }

    PlayerPacketInput formPacket(){
        PlayerPacketInput PP;
        PP.prev_x = prev_x;
        PP.prev_y = prev_y;
        PP.x = x;
        PP.y = y;
        PP.character = character;
        return PP;
    }

    bool fix(const PlayerPacketOutput& PP){
        if(abs(PP.x - x) > 1 || abs(PP.y - y) > 1 || abs(PP.x - x) >=1  && abs(PP.y - y) >= 1){
             return false;
        }

        //fix player coords with verified ones
        prev_x = PP.prev_x;
        prev_y = PP.prev_y;
        x = PP.x;
        y = PP.y;
        character = PP.character;
        return true;
    }

    uint8_t safe_add(uint8_t a, uint8_t b) {
    if ((int)a + b > 255) {
        return 255;
    }
    return a + b;
    }

    uint8_t safe_sub(uint8_t a, uint8_t b) {
        if (b > a) {
            return 0; 
        }
        return a - b;
    }

    bool checkLegal(uint8_t x, uint8_t y){
        if(y != prev_y && x != prev_x){
            return false;
        }
        if(y < 0 || y > 254 || abs(y - prev_y) > 1){
            return false;    
        }
        if(x < 0 || x > 254 || abs(x - prev_x) > 1){
            return false;    
        }
        if(full_map[y*254 + x] == '*'){
            return false;    
        } 
        return true;
    }

    PlayerState checkDead(const PlayerPacketOutput& PP){
        for(const auto& enemy_pos: PP.enemy_positions){
            if(enemy_pos.x == x && enemy_pos.y == y){
               return PlayerState::Dead; 
            }
        }
        return PlayerState::Alive;
    }

    void init(uint8_t x_, uint8_t y_, char character_){
        x = x_;
        y = y_;
        prev_x = x_;
        prev_y = y_;
        character = character_;
    }

private:
    uint8_t x=2;
    uint8_t y=2;
    uint8_t prev_x = 2;
    uint8_t prev_y = 2;
    char  character = 'h';
    std::vector<char>full_map;
    uint8_t seq_num=0;
};