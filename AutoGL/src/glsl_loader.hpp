// src/glsl_loader.hpp
#pragma once
#include <string>
#include <glad/glad.h>

namespace AutoGL {

    // 전체 GLSL 파일을 파싱하여 프로그램 생성
    GLuint loadShaderProgram(const std::string& path);

    // 내부용 헬퍼 (파일 읽기)
    std::string loadFileSource(const std::string& path);

    // 기존 API 호환용, 내부적으로 GL::CompileShaderSource 사용
    GLuint compileShader(GLenum type, const std::string& source);

} // namespace AutoGL
