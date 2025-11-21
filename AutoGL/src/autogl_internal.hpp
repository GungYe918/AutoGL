// src/autogl_internal.hpp
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace AutoGL {

    struct InternalGLState {
        GLFWwindow* window = nullptr;
        int width  = 800;
        int height = 600;

        // fullscreen quad
        unsigned int quadVAO = 0;
        unsigned int quadVBO = 0;

        // time
        double startTime     = 0.0;
        double prevFrameTime = 0.0;
        double deltaTime     = 0.0;
        int    frameCount    = 0;

        // random
        float randomValue = 0.0f;

        // mouse
        double mouseX   = 0.0;
        double mouseY   = 0.0;
        bool   mouseDown = false;

        double clickX = 0.0;
        double clickY = 0.0;

        // shadertoy style channels
        unsigned int textures[4] = {0, 0, 0, 0};
        int texWidth[4]          = {0, 0, 0, 0};
        int texHeight[4]         = {0, 0, 0, 0};
        double channelTime[4]    = {0.0, 0.0, 0.0, 0.0};

        // SSBO handles (for compute)
        std::vector<unsigned int> ssboList;
    };

} // namespace AutoGL
