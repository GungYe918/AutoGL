// src/vk_engine.cpp
#include "vk_engine.hpp"
#include <AutoGL/Log.hpp>

namespace AutoGL {

    bool EngineVKBackend::init() {
        AUTOGL_LOG_ERROR("EngineVK", "Vulkan backend not implemented yet");
        return false;
    }

    void EngineVKBackend::setWindowSize(int, int) {
        // not implemented
    }

    void EngineVKBackend::mainLoop(const std::string&) {
        AUTOGL_LOG_ERROR("EngineVK", "Vulkan backend not implemented yet");
    }

    bool EngineVKBackend::runShaderFile(const std::string&) {
        AUTOGL_LOG_ERROR("EngineVK", "Vulkan backend not implemented yet");
        return false;
    }

} // namespace AutoGL
