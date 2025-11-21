// include/AutoGL/AutoGL.hpp
#pragma once
#include <string>
#include <vector>

namespace AutoGL {

    struct ShaderFile {
        std::string path;
        std::string source;
    };

    enum class BackendAPI {
        OpenGL,
        Vulkan
    };

    class Engine {
    public:
        // 기본은 OpenGL 백엔드로 동작
        explicit Engine(BackendAPI api = BackendAPI::OpenGL);
        ~Engine();

        // 기존 API 유지
        bool initGL();
        void mainLoop(const std::string& shaderPath);
        void setWindowSize(int w, int h);

        std::vector<ShaderFile> scanShaderFolder(const std::string& folder);
        bool runShaderFile(const std::string& path);

        BackendAPI backend() const noexcept;

    private:
        struct Impl;
        Impl* pimpl;
    };

} // namespace AutoGL
