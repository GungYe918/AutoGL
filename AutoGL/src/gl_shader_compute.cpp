// src/gl_shader_compute.cpp
#include "gl_shader_compute.hpp"
#include "gl_shader_common.hpp"

#include <AutoGL/Log.hpp>

namespace AutoGL::GL {

    GLuint CompileComputeShader(const std::string& source) {
        AUTOGL_LOG_INFO("GLSL", "Compiling compute shader");

        return CompileShaderSource(
            ShaderStage::Compute,
            source,
            "compute"
        );
    }

}
