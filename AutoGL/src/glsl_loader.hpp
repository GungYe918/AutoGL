#pragma once
#include <string>
#include <glad/glad.h>

namespace AutoGL {

    GLuint loadShaderProgram(const std::string& path);

    // 내부 함수
    std::string loadFileSource(const std::string& path);
    GLuint compileShader(GLenum type, const std::string& source);

}
