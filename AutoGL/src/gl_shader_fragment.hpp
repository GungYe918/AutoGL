// src/gl_shader_fragment.hpp
#pragma once
#include <string>
#include <glad/glad.h>

namespace AutoGL::GL {

    GLuint CompileFragmentShader(const std::string& source);

}
