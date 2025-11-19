// src/autogl_internal.hpp
#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace AutoGL {

    struct InternalGLState {
        GLFWwindow* window = nullptr;
        int width = 800;
        int height = 600;

        GLuint quadVAO = 0;
        GLuint quadVBO = 0;
    };

}
