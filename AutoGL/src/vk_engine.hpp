// src/vk_engine.hpp
#pragma once
#include "engine_backend.hpp"

namespace AutoGL {

    class EngineVKBackend final : public EngineBackend {
    public:
        EngineVKBackend() = default;
        ~EngineVKBackend() override = default;

        bool init() override;
        void setWindowSize(int, int) override;

        void mainLoop(const std::string& shaderPath) override;
        bool runShaderFile(const std::string& path) override;
    };

} // namespace AutoGL
