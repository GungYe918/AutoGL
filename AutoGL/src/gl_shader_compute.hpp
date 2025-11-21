// src/gl_shader_compute.hpp
#pragma once
#include <string>
#include <glad/glad.h>

namespace AutoGL::GL {

    GLuint CompileComputeShader(const std::string& source);

}
