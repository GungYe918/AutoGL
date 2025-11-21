// src/autogl_internal.hpp
#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace AutoGL {

    struct InternalGLState {
        GLFWwindow* window = nullptr;
        int width = 800;
        int height = 600;

        // Fullscreen quad
        GLuint quadVAO = 0;
        GLuint quadVBO = 0;

        // Time
        double startTime = 0.0;
        double prevFrameTime = 0.0;
        double deltaTime = 0.0;
        int frameCount = 0;

        // Random
        float randomValue = 0.0f;

        // Mouse
        double mouseX = 0.0;
        double mouseY = 0.0;
        bool mouseDown = false;

        double clickX = 0.0;
        double clickY = 0.0;

        // Texture channels (ShaderToy style)
        // iChannel0 ... iChannel3
        GLuint textures[4] = {0, 0, 0, 0};

        // Texture resolution
        int texWidth[4]  = {0, 0, 0, 0};
        int texHeight[4] = {0, 0, 0, 0};

        // Time per texture channel
        double channelTime[4] = {0.0, 0.0, 0.0, 0.0};

        // If SSBO auto creation is needed for compute shaders
        // we will store handles here
        std::vector<GLuint> ssboList;
    };

}
