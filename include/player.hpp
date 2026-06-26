#include <array>
#include <cstdint>
#include <iostream>
#include <ncurses.h>

enum class PlayerState {
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
};

struct PlayerPacketOutput {
    uint8_t player_id;
    std::vector<EnemyInfo> enemy_positions;
    uint8_t x;
    uint8_t y;
    uint8_t prev_x;
    uint8_t prev_y;
    char character;
    PlayerState PS;
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
                y += 1; // Move Up
                break;
            case 's':
            case 'S':
                y -= 1; // Move Down
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
    int x=0;
    int y=0;
    int prev_x = 0;
    int prev_y= 0;
    char  character = 'h';
    std::vector<char>full_map;
};
