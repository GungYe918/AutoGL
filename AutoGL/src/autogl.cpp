// src/autogl.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "autogl_internal.hpp"
#include "glsl_loader.hpp"
#include <AutoGL/AutoGL.hpp>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace AutoGL::detail {

    void createFullscreenQuad(InternalGLState& state) {
        float vertices[] = {
            -1.f,-1.f,  0.f,0.f,
             1.f,-1.f,  1.f,0.f,
             1.f, 1.f,  1.f,1.f,
            -1.f,-1.f,  0.f,0.f,
             1.f, 1.f,  1.f,1.f,
            -1.f, 1.f,  0.f,1.f
        };

        glGenVertexArrays(1, &state.quadVAO);
        glGenBuffers(1, &state.quadVBO);

        glBindVertexArray(state.quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, state.quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }



} // namespace AutoGL::detail

namespace AutoGL {

    struct Engine::Impl {
        InternalGLState state;

        GLuint currentProgram = 0;

        bool initGL();
        GLuint tryLoadProgram(const std::string& path);
        void swapProgram(GLuint newProg);
    };

    Engine::Engine() : pimpl(new Impl()) {}
    Engine::~Engine() { delete pimpl; }

    bool Engine::initGL() {
        return pimpl->initGL();
    }

    bool Engine::Impl::initGL() {
        if (!glfwInit()) {
            std::cerr << "GLFW init 실패\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        state.window = glfwCreateWindow(state.width, state.height, "AutoGL Hot Reload", 0, 0);
        if (!state.window) {
            std::cerr << "GLFW window 생성 실패\n";
            return false;
        }
        glfwMakeContextCurrent(state.window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "GLAD load 실패\n";
            return false;
        }

        detail::createFullscreenQuad(state);

        glClearColor(0.0f, 0.0f, 0.0f, 1.f);

        return true;
    }


    bool Engine::runShaderFile(const std::string& path) {
        GLuint program = loadShaderProgram(path);
        if (program == 0) return false;

        glUseProgram(program);

        auto& st = pimpl->state;

        // 기본 uniform
        GLint uTime = glGetUniformLocation(program, "iTime");
        GLint uRes  = glGetUniformLocation(program, "iResolution");

        double start = glfwGetTime();

        while (!glfwWindowShouldClose(st.window)) {
            float time = glfwGetTime() - start;
            int width, height;
            glfwGetFramebufferSize(st.window, &width, &height);

            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);

            if (uTime >= 0) glUniform1f(uTime, time);
            if (uRes  >= 0) glUniform2f(uRes, (float)width, (float)height);

            glBindVertexArray(st.quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapBuffers(st.window);
            glfwPollEvents();
        }

        glDeleteProgram(program);
        return true;
    }



    std::vector<ShaderFile> Engine::scanShaderFolder(const std::string& folder) {
        std::vector<ShaderFile> shaders;

        if (!fs::exists(folder)) {
            std::cerr << "[AutoGL] Shader 폴더 없음: " << folder << "\n";
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
                    std::cout << "[AutoGL] 발견한 shader: " << sf.path << "\n";
                }
            }
        }

        return shaders;
    }

    GLuint Engine::Impl::tryLoadProgram(const std::string& path) {
        GLuint p = loadShaderProgram(path);
        if (p == 0) {
            std::cerr << "[AutoGL] Shader 컴파일 실패 → 기존 프로그램 유지\n";
            return 0;
        }
        return p;
    }

    void Engine::Impl::swapProgram(GLuint newProg) {
        if (newProg == 0)
            return;

        if (currentProgram != 0)
            glDeleteProgram(currentProgram);

        currentProgram = newProg;
        glUseProgram(currentProgram);
    }

    void Engine::mainLoop(const std::string& shaderPath) {
        auto& st = pimpl->state;

        fs::file_time_type lastTime = fs::last_write_time(shaderPath);

        GLuint initial = pimpl->tryLoadProgram(shaderPath);
        pimpl->swapProgram(initial);

        double startTime = glfwGetTime();

        while (!glfwWindowShouldClose(st.window)) {

            // 1) 파일 변경 감지
            auto now = fs::last_write_time(shaderPath);
            if (now != lastTime) {
                lastTime = now;
                std::cout << "[AutoGL] Shader 변경 감지 → 재컴파일!\n";

                GLuint np = pimpl->tryLoadProgram(shaderPath);
                pimpl->swapProgram(np);
            }

            // 2) Uniform 업데이트
            int w, h;
            glfwGetFramebufferSize(st.window, &w, &h);
            glViewport(0, 0, w, h);

            glClear(GL_COLOR_BUFFER_BIT);

            if (pimpl->currentProgram) {
                double t = glfwGetTime() - startTime;

                GLint uTime = glGetUniformLocation(pimpl->currentProgram, "iTime");
                GLint uRes  = glGetUniformLocation(pimpl->currentProgram, "iResolution");

                if (uTime >= 0) glUniform1f(uTime, (float)t);
                if (uRes  >= 0) glUniform2f(uRes, (float)w, (float)h);

                glBindVertexArray(st.quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glfwSwapBuffers(st.window);
            glfwPollEvents();
        }
    }



}

