#include "GuiApp.hpp"
#include "ImGuiLayer.hpp"

#include <iostream>

void GuiApp::initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to init glfw" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    window = glfwCreateWindow(1920, 1080, "AutoGL ImGui test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create Window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

void GuiApp::shutdownWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GuiApp::run() {
    initWindow();
    if (!window) {  return;  }

    
}