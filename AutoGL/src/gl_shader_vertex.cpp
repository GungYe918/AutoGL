// src/gl_shader_vertex.cpp
#include "gl_shader_vertex.hpp"
#include "gl_shader_common.hpp"

namespace AutoGL::GL {

    GLuint CompileVertexShader(const std::string& source) {
        return CompileShaderSource(ShaderStage::Vertex, source, "vertex");
    }

}
