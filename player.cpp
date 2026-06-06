#include <array>
#include <cstdint>
#include <GLFW/glfw3.h>
#include <iostream>


class Player {
public:
    // Default constructor zeroes out the array automatically
    Player() = default;

    void processInput(GLFWwindow* window) {
    // Check if key is currently held down
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) y += 1; // Move Up
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) y -= 1; // Move Down
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) x -= 1; // Move Left
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) x += 1; // Move Right
    }

    std::array<uint8_t, 3> get_player_coords() const {
        return coords;
    }

    void set_player_coords(const std::array<uint8_t, 3>& arr) {
        coords = arr; 
    }

private:
    // Value-initialization {} ensures x, y, and z all start at 0
    std::array<uint8_t, 3> coords{}; 
};
