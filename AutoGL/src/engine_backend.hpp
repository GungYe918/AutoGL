// src/engine_backend.hpp
#pragma once
#include <string>
#include <AutoGL/AutoGL.hpp>

namespace AutoGL {

    class EngineBackend {
    public:
        virtual ~EngineBackend() = default;

        virtual bool init() = 0;
        virtual void setWindowSize(int w, int h) = 0;

        virtual void mainLoop(const std::string& shaderPath) = 0;
        virtual bool runShaderFile(const std::string& path) = 0;
    };

} // namespace AutoGL