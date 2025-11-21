// src/gl_shader_vertex.hpp
#pragma once
#include <string>
#include <glad/glad.h>

namespace AutoGL::GL {

    GLuint CompileVertexShader(const std::string& source);

}
