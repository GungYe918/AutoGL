// src/gl_shader_fragment.cpp
#include "gl_shader_fragment.hpp"
#include "gl_shader_common.hpp"

namespace AutoGL::GL {

    GLuint CompileFragmentShader(const std::string& source) {
        return CompileShaderSource(ShaderStage::Fragment, source, "fragment");
    }

}
