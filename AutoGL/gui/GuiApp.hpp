#pragma once
#include <GLFW/glfw3.h>

class GuiApp {
public:
    void run();
    
private:
    GLFWwindow* window = nullptr;

    void initWindow();
    void shutdownWindow();
};