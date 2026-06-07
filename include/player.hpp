#include <array>
#include <cstdint>
#include <GLFW/glfw3.h>
#include <iostream>

enum class PlayerState {
    Alive,    
    Dead,
};

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
    Player() = default;

    void processInput(GLFWwindow* window) {
        prev_x = x;
        prev_y = y;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) y += 1; // Move Up
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) y -= 1; // Move Down
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) x -= 1; // Move Left
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) x += 1; // Move Right
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

    void render(const PlayerPacketOutput& PP){
        
    }




    void set_player_coords(const std::array<uint8_t, 3>& arr) {
        coords = arr; 
    }

private:
    int x=0;
    int y=0;
    int prev_x = 0;
    int prev_y= 0;
    char  character = 'h';
    std::vector<char>full_map;
};
