#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <ncurses.h>

enum class PlayerState:uint8_t {
    Alive,    
    Dead,
};

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
    uint8_t prev_x;
    uint8_t prev_y;
    uint8_t x;
    uint8_t y;
    char character;
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer;
        buffer.push_back(x);
        buffer.push_back(y);
        buffer.push_back(prev_x);
        buffer.push_back(prev_y);
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
        PP.character = static_cast<char>(buffer[offset++]);
        return PP;
    }
};

struct PlayerPacketOutput {
public:
    uint8_t player_id;
    std::vector<EnemyInfo> enemy_positions;
    uint8_t x;
    uint8_t y;
    uint8_t prev_x;
    uint8_t prev_y;
    char character;
    PlayerState PS;

    static std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer;
        buffer.push_back(player_id);
        buffer.push_back(x);
        buffer.push_back(y);
        buffer.push_back(prev_x);
        buffer.push_back(prev_y);
        buffer.push_back(static_cast<uint8_t>(character));
        buffer.push_back(static_cast<uint8_t>(PS));
        uint8_t enemy_count = static_cast<uint8_t>(enemy_positions.size());
        buffer.push_back(enemy_count);
        for (const auto& enemy : enemy_positions) {
            buffer.push_back(static_cast<uint8_t>(enemy.character));
            buffer.push_back(enemy.x);
            buffer.push_back(enemy.y);
        }
        return buffer;
    }

    PlayerPacketOutput deserialize(const std::vector<uint8_t>& buffer){
        PlayerPacketOutput PP;
        size_t offset = 0;
        PP.player_id = buffer[offset++];
        PP.x = buffer[offset++];
        PP.y = buffer[offset++];
        PP.prev_x = buffer[offset++];
        PP.prev_y = buffer[offset++];
        PP.character = static_cast<char>(buffer[offset++]);
        PP.PS = static_cast<PlayerState>(buffer[offset++]);
        uint8_t enemy_count = buffer[offset++];
        for (int i = 0; i < enemy_count; ++i) {
            EnemyInfo enemy;
            enemy.character = static_cast<char>(buffer[offset++]);
            enemy.x  = buffer[offset++];
            enemy.y  = buffer[offset++];
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
    }

    ~Player(){
        endwin(); 
    }

    void processInput() {
        // Store the previous position
        prev_x = x;
        prev_y = y;

        // Read a single character from the terminal input buffer
        int ch = getch(); 

        // Process the key press
        switch (ch) {
            case 'w':
            case 'W':
                y -= 1; // Move Up
                break;
            case 's':
            case 'S':
                y += 1; // Move Down
                break;
            case 'a':
            case 'A':
                x -= 1; // Move Left
                break;
            case 'd':
            case 'D':
                x += 1; // Move Right
                break;
            default:
                // No movement or unhandled key
                break;
        }
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

private:
    int x=2;
    int y=2;
    int prev_x = 0;
    int prev_y= 1;
    char  character = 'h';
};
