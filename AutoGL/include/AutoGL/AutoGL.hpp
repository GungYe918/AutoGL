// include/AutoGL/AutoGL.hpp
#pragma once
#include <string>
#include <vector>

namespace AutoGL {

    struct ShaderFile {
        std::string path;
        std::string source;
    };

    class Engine {
    public:
        Engine();
        ~Engine();

        bool initGL();
        void mainLoop(const std::string& shaderPath);
        void setWindowSize(int w, int h);

        // shader/ 폴더 자동 스캔
        std::vector<ShaderFile> scanShaderFolder(const std::string& folder);

        // GLSL 파일 컴파일 & 렌더 수행
        bool runShaderFile(const std::string& path);

        // compute shader 실행 (중급 난이도)
        bool runComputeShader(const std::string& path);

    private:
        struct Impl;
        Impl* pimpl;
    };

}
