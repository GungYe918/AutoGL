// src/gl_shader_common.hpp
#pragma once
#include <string>
#include <glad/glad.h>

namespace AutoGL::GL {

    enum class ShaderStage {
        Vertex,
        Fragment,
        Compute
    };

    // 공통 컴파일 함수
    GLuint CompileShaderSource(
        ShaderStage stage,
        const std::string& source,
        const char* debugName
    );

} // namespace AutoGL::GL
