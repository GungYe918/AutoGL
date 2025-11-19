#include "glsl_loader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <regex>

namespace AutoGL::detail {

    // -----------------------------------------------------
    // @type vertex
    // @type fragment
    // @type compute
    //
    // 섹션별 source를 map 형태로 추출하는 함수
    // -----------------------------------------------------
    static bool extractShaderSections(
        const std::string& full,
        std::string& outVert,
        std::string& outFrag,
        std::string& outComp
    ) {
        const std::string token = "@type ";

        // glsl파일 안에서 @type 문자열 검색
        std::vector<size_t> markers;
        size_t pos = full.find(token);
        while (pos != std::string::npos) {
            markers.push_back(pos);
            pos = full.find(token, pos + token.size());
        }

        if (markers.empty()) {
            std::cerr << "[GLSL Loader] @type 구문을 찾지 못함\n";
            return false;
        }

        // 마지막 토큰 뒤의 끝 위치를 위해 sentinel 추가
        markers.push_back(full.size());

        for (size_t i = 0; i < markers.size() - 1; i++) {
            size_t start = markers[i];
            size_t next  = markers[i + 1];

            // 라인 파싱 -> "@type vertex", "@type fragment", "@type compute"
            size_t lineEnd = full.find('\n', start);
            if (lineEnd == std::string::npos) {
                lineEnd = next;
            }

            std::string header = full.substr(start, lineEnd - start);

            std::string shaderCode = full.substr(lineEnd, next - lineEnd);

            // header 분석
            if (header.find("@type vertex") != std::string::npos) {
                outVert = shaderCode;
            }
            else if (header.find("@type fragment") != std::string::npos) {
                outFrag = shaderCode;
            }
            else if (header.find("@type compute") != std::string::npos) {
                outComp = shaderCode;
            }
            else {
                std::cerr << "[GLSL Loader] 알 수 없는 @type 토큰: "
                          << header << "\n";
            }
        }

        return true;
    }

    static std::vector<int> scanSSBOBindings(const std::string& source) {
        std::vector<int> result;

        std::regex r("layout\\s*\\(.*binding\\s*=\\s*(\\d+)\\s*\\).*buffer");
        std::smatch m;

        std::string s = source;
        while (std::regex_search(s, m, r)) {
            int b = std::stoi(m[1].str());
            result.push_back(b);
            s = m.suffix();
        }

        return result;
    }

    GLuint createEmptySSBO(size_t size) {
        GLuint ssbo = 0;
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return ssbo;
    }

} // namespace AutoGL::detail

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

    // -----------------------------------------------------
    // loadShaderProgram
    // vertex + fragment + compute
    // -----------------------------------------------------
    GLuint loadShaderProgram(const std::string& path) {
        std::string full = loadFileSource(path);
        if (full.empty()) return 0;

        std::string vsrc, fsrc, csrc;
        if (!detail::extractShaderSections(full, vsrc, fsrc, csrc)) {
            return 0;
        }

        if (fsrc.empty() && csrc.empty()) {
            std::cerr << "[GLSL Loader] fragment or compute needed\n";
            return 0;
        }

        GLuint vert = 0, frag = 0, comp = 0;

        if (!vsrc.empty()) {
            vert = compileShader(GL_VERTEX_SHADER, vsrc);
            if (vert == 0) return 0;
        }
        if (!fsrc.empty()) {
            frag = compileShader(GL_FRAGMENT_SHADER, fsrc);
            if (frag == 0) {
                if (vert) glDeleteShader(vert);
                return 0;
            }
        }
        if (!csrc.empty()) {
            comp = compileShader(GL_COMPUTE_SHADER, csrc);
            if (comp == 0) {
                if (vert) glDeleteShader(vert);
                if (frag) glDeleteShader(frag);
                return 0;
            }
        }

        GLuint program = glCreateProgram();
        if (vert) glAttachShader(program, vert);
        if (frag) glAttachShader(program, frag);
        if (comp) glAttachShader(program, comp);

        glLinkProgram(program);

        GLint ok;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[2048];
            glGetProgramInfoLog(program, 2048, nullptr, log);
            std::cerr << "[GLSL Loader] link fail\n" << log << "\n";

            glDeleteProgram(program);
            if (vert) glDeleteShader(vert);
            if (frag) glDeleteShader(frag);
            if (comp) glDeleteShader(comp);
            return 0;
        }

        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        if (comp) glDeleteShader(comp);

        // ---------------------------------------------
        // SSBO auto creation and binding for compute
        // ---------------------------------------------
        if (!csrc.empty()) {

            std::vector<int> ssboList = detail::scanSSBOBindings(csrc);

            for (int b : ssboList) {

                size_t bufSize = 4096;     // default size, will be expanded later
                GLuint ssbo = detail::createEmptySSBO(bufSize);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, b, ssbo);

                std::cout << "[GLSL Loader] SSBO created for binding "
                        << b << " (size " << bufSize << ")\n";
            }
        }

        return program;
    }

}
