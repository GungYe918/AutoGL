// src/gl_shader_common.cpp
#include "gl_shader_common.hpp"
#include <AutoGL/Log.hpp>
#include <vector>

namespace AutoGL::GL {

    namespace {
        constexpr GLenum toGLenum(ShaderStage stage) {
            switch (stage) {
                case ShaderStage::Vertex:   return GL_VERTEX_SHADER;
                case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
                case ShaderStage::Compute:  return GL_COMPUTE_SHADER;
            }
            return GL_VERTEX_SHADER;
        }
    }

    GLuint CompileShaderSource(ShaderStage stage,
                               const std::string& source,
                               const char* debugName) {
        const GLenum glType = toGLenum(stage);
        GLuint shader = glCreateShader(glType);
        if (!shader) {
            AUTOGL_LOG_ERROR("GLShader", "glCreateShader failed");
            return 0;
        }

        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[2048];
            glGetShaderInfoLog(shader, 2048, nullptr, log);
            AUTOGL_LOG_ERROR("GLShader",
                std::string("Shader compile failed in ")
                + (debugName ? debugName : "unknown") + ": " + log);
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

} // namespace AutoGL::GL
