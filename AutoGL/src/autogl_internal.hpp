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

        GLuint quadVAO = 0;
        GLuint quadVBO = 0;

        double startTime = 0.0f;
        
        // 마우스 상태
        double mouseX = 0.0;
        double mouseY = 0.0;
        bool mouseDown = false;

        // 클릭 시점 좌표 (ShaderToy 호환)
        double clickX = 0.0;
        double clickY = 0.0;

        // texture slots
        std::vector<GLuint> textures;
    };

}
