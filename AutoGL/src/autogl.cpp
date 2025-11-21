// src/autogl.cpp
#include <AutoGL/AutoGL.hpp>
#include <AutoGL/Log.hpp>

#include "engine_backend.hpp"
#include "gl_engine.hpp"
#include "vk_engine.hpp"
#include "glsl_loader.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace AutoGL {

    struct Engine::Impl {
        BackendAPI api = BackendAPI::OpenGL;
        EngineBackend* backend = nullptr;

        Impl(BackendAPI apiIn)
            : api(apiIn) {
            switch (api) {
                case BackendAPI::OpenGL:
                    backend = new EngineGLBackend();
                    break;
                case BackendAPI::Vulkan:
                    backend = new EngineVKBackend();
                    break;
            }
        }

        ~Impl() {
            delete backend;
        }
    };

    Engine::Engine(BackendAPI api)
        : pimpl(new Impl(api)) {
        if (api == BackendAPI::Vulkan) {
            AUTOGL_LOG_WARN("Engine",
                "Vulkan backend requested but not implemented, "
                "methods will fail");
        }
    }

    Engine::~Engine() {
        delete pimpl;
        pimpl = nullptr;
    }

    bool Engine::initGL() {
        if (!pimpl || !pimpl->backend) {
            return false;
        }
        // 현재는 OpenGL 전용으로 사용
        return pimpl->backend->init();
    }

    void Engine::mainLoop(const std::string& shaderPath) {
        if (!pimpl || !pimpl->backend) return;
        pimpl->backend->mainLoop(shaderPath);
    }

    void Engine::setWindowSize(int w, int h) {
        if (!pimpl || !pimpl->backend) return;
        pimpl->backend->setWindowSize(w, h);
    }

    bool Engine::runShaderFile(const std::string& path) {
        if (!pimpl || !pimpl->backend) return false;
        return pimpl->backend->runShaderFile(path);
    }

    std::vector<ShaderFile> Engine::scanShaderFolder(const std::string& folder) {
        std::vector<ShaderFile> shaders;

        if (!fs::exists(folder)) {
            AUTOGL_LOG_WARN("Engine", "shader folder not found " + folder);
            return shaders;
        }

        for (auto& entry : fs::directory_iterator(folder)) {
            if (!entry.is_regular_file()) continue;

            auto path = entry.path();
            if (path.extension() == ".glsl") {
                ShaderFile sf;
                sf.path = path.string();
                sf.source = loadFileSource(sf.path);

                if (!sf.source.empty()) {
                    shaders.push_back(sf);
                    AUTOGL_LOG_INFO("Engine",
                        "found shader " + sf.path);
                }
            }
        }

        return shaders;
    }

    BackendAPI Engine::backend() const noexcept {
        if (!pimpl) return BackendAPI::OpenGL;
        return pimpl->api;
    }

} // namespace AutoGL
