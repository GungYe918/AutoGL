#include "glsl_loader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace AutoGL {

    std::string loadFileSource(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[GLSL Loader] 파일 열기 실패: " << path << "\n";
            return "";
        }

        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    GLuint compileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();

        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        // 컴파일 상태 체크
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            char log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            std::cerr << "[GLSL Loader] 쉐이더 컴파일 실패:\n" << log << "\n";
        }

        return shader;
    }

    GLuint loadShaderProgram(const std::string& path) {
        // GLSL 파일 형식:
        // #type vertex
        // ... vertex shader ...
        // #type fragment
        // ... fragment shader ...

        std::string full = loadFileSource(path);
        if (full.empty()) return 0;

        // vertex, fragment 두 파트를 분리
        const std::string token = "#type ";
        size_t posV = full.find("#type vertex");
        size_t posF = full.find("#type fragment");

        if (posV == std::string::npos || posF == std::string::npos) {
            std::cerr << "[GLSL Loader] GLSL 파일 형식 오류\n";
            return 0;
        }

        std::string vsrc = full.substr(
            posV + token.size() + 6,
            posF - (posV + token.size() + 6)
        );
        std::string fsrc = full.substr(posF + token.size() + 8);

        GLuint vs = compileShader(GL_VERTEX_SHADER, vsrc);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc);

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        // 링크 체크
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            char log[1024];
            glGetProgramInfoLog(program, 1024, nullptr, log);
            std::cerr << "[GLSL Loader] 프로그램 링크 실패:\n" << log << "\n";
        }

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

}
