// src/engine_gl.hpp
#pragma once
#include "engine_backend.hpp"
#include "autogl_internal.hpp"

namespace AutoGL {

    class EngineGLBackend final : public EngineBackend {
    public:
        EngineGLBackend();
        ~EngineGLBackend() override;

        bool init() override;
        void setWindowSize(int w, int h) override;

        void mainLoop(const std::string& shaderPath) override;
        bool runShaderFile(const std::string& path) override;

    private:
        InternalGLState state_;
        unsigned int currentProgram_ = 0;

        bool initContext();
        unsigned int tryLoadProgram(const std::string& path);
        void swapProgram(unsigned int newProgram);
    };

} // namespace AutoGL
