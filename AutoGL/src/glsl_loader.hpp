// src/glsl_loader.hpp
#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>

#include "glsl_types.hpp"


namespace AutoGL {

    struct LoadedShaderProgram {
        GLuint program = 0;
        // Type 파싱 수행
        std::unordered_map<int, SSBOTypeInfo> bindingTypeInfo;
    };

    // 전체 GLSL 파일을 파싱하여 프로그램 생성
    LoadedShaderProgram loadShaderProgram(const std::string& path);

    // 내부용 헬퍼 (파일 읽기)
    std::string loadFileSource(const std::string& path);

    // 기존 API 호환용, 내부적으로 GL::CompileShaderSource 사용
    GLuint compileShader(GLenum type, const std::string& source);

} // namespace AutoGL
