#pragma once
#include <GLFW/glfw3.h> 

class ImGuiLayer {
public: 
    void init(GLFWwindow* window);
    void beginFrame();
    void endFrame();
    void shutdown();
};