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

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        auto* st = (AutoGL::InternalGLState*)glfwGetWindowUserPointer(window);
        if (!st) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                st->mouseDown = true;
                double cx, cy;
                glfwGetCursorPos(window, &cx, &cy);

                int w, h;
                glfwGetFramebufferSize(window, &w, &h);

                st->clickX = cx;
                st->clickY = h - cy;
            } else if (action == GLFW_RELEASE) {
                st->mouseDown = false;
            }
        }
    }

    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
        auto* st = (AutoGL::InternalGLState*)glfwGetWindowUserPointer(window);  

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);


        st->mouseX = xpos;
        st->mouseY = h - ypos;
    }

    static void setBuiltinUniforms(GLuint program, InternalGLState& st) {
        GLenum err;

        // Time
        float timeNow = (float)(glfwGetTime() - st.startTime);

        GLint uTime = glGetUniformLocation(program, "iTime");
        if (uTime >= 0) {
            glUniform1f(uTime, timeNow);
        }

        // TimeDelta
        double now = glfwGetTime();
        st.deltaTime = now - st.prevFrameTime;
        st.prevFrameTime = now;

        GLint uTimeDelta = glGetUniformLocation(program, "iTimeDelta");
        if (uTimeDelta >= 0) {
            glUniform1f(uTimeDelta, (float)st.deltaTime);
        }

        // Frame
        st.frameCount++;
        GLint uFrame = glGetUniformLocation(program, "iFrame");
        if (uFrame >= 0) {
            glUniform1i(uFrame, st.frameCount);
        }

        // GlobalTime (alias)
        GLint uGlobalTime = glGetUniformLocation(program, "iGlobalTime");
        if (uGlobalTime >= 0) {
            glUniform1f(uGlobalTime, timeNow);
        }

        // Resolution
        int w, h;
        glfwGetFramebufferSize(st.window, &w, &h);
        GLint uRes = glGetUniformLocation(program, "iResolution");
        if (uRes >= 0) {
            glUniform2f(uRes, (float)w, (float)h);
        }

        // Mouse
        GLint uMouse = glGetUniformLocation(program, "iMouse");
        if (uMouse >= 0) {
            glUniform4f(
                uMouse,
                (float)st.mouseX,
                (float)st.mouseY,
                st.mouseDown ? (float)st.clickX : 0.f,
                st.mouseDown ? (float)st.clickY : 0.f
            );
        }

        // Date (year, month, day, seconds)
        {
            time_t t = time(nullptr);
            tm* lt = localtime(&t);

            float seconds = lt->tm_hour * 3600.0f +
                            lt->tm_min * 60.0f +
                            lt->tm_sec;

            GLint uDate = glGetUniformLocation(program, "iDate");
            if (uDate >= 0) {
                glUniform4f(
                    uDate,
                    (float)(lt->tm_year + 1900),
                    (float)(lt->tm_mon + 1),
                    (float)(lt->tm_mday),
                    seconds
                );
            }
        }

        // FrameRate (approx)
        float frameRate = (st.deltaTime > 0.0) ? (float)(1.0 / st.deltaTime) : 0.0f;
        GLint uRate = glGetUniformLocation(program, "iFrameRate");
        if (uRate >= 0) {
            glUniform1f(uRate, frameRate);
        }

        // Random
        st.randomValue = (float)rand() / (float)RAND_MAX;
        GLint uRand = glGetUniformLocation(program, "iRandom");
        if (uRand >= 0) {
            glUniform1f(uRand, st.randomValue);
        }

        // Texture channel sizes (if channels exist later)
        for (int i = 0; i < 4; i++) {
            std::string name = "iChannelResolution[" + std::to_string(i) + "]";
            GLint loc = glGetUniformLocation(program, name.c_str());
            if (loc >= 0) {
                glUniform3f(loc, (float)st.texWidth[i], (float)st.texHeight[i], 1.0f);
            }

            std::string tname = "iChannelTime[" + std::to_string(i) + "]";
            GLint tloc = glGetUniformLocation(program, tname.c_str());
            if (tloc >= 0) {
                glUniform1f(tloc, (float)(glfwGetTime() - st.channelTime[i]));
            }
        }

        // Check errors
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "GL error in setBuiltinUniforms: " << err << "\n";
        }
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

        state.window = glfwCreateWindow(state.width, state.height, "AutoGL Engine", 0, 0);
        if (!state.window) {
            std::cerr << "GLFW window 생성 실패\n";
            return false;
        }
        glfwMakeContextCurrent(state.window);
        glfwSetWindowUserPointer(state.window, &state);

        glfwSetCursorPosCallback(state.window, detail::cursor_pos_callback);
        glfwSetMouseButtonCallback(state.window, detail::mouse_button_callback);

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

        auto& st = pimpl->state;

        glUseProgram(program);

        // compute shader 감지
        GLint numShaders = 0;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &numShaders);
        std::vector<GLuint> attached(numShaders);
        glGetAttachedShaders(program, numShaders, nullptr, attached.data());

        bool hasCompute = false;
        for (GLuint s : attached) {
            GLint type = 0;
            glGetShaderiv(s, GL_SHADER_TYPE, &type);
            if (type == GL_COMPUTE_SHADER) {
                hasCompute = true;
            }
        }

        st.startTime = glfwGetTime();

        while (!glfwWindowShouldClose(st.window)) {

            glUseProgram(program);
            detail::setBuiltinUniforms(program, st);

            if (hasCompute) {
                glDispatchCompute(8, 8, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            } else {
                glBindVertexArray(st.quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glfwSwapBuffers(st.window);
            glfwPollEvents();

            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "GL error in frame loop: " << err << "\n";
            }
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
            std::cerr << "[AutoGL] Shader 컴파일 실패 -> 기존 프로그램 유지\n";
            return 0;
        }
        return p;
    }

    void Engine::Impl::swapProgram(GLuint newProg) {
        if (newProg == 0) {
            return;
        }
        
        if (currentProgram != 0) {
            glDeleteProgram(currentProgram);
            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "GL error after deleting program: " << err << "\n";
            }
        }

        currentProgram = newProg;
        glUseProgram(currentProgram);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "GL error after using new program: " << err << "\n";
        }
    }

    // mainLoop: shader hot reload loop
    void Engine::mainLoop(const std::string& shaderPath) {
        auto& st = pimpl->state;

        fs::file_time_type lastTime = fs::last_write_time(shaderPath);

        GLuint initial = pimpl->tryLoadProgram(shaderPath);
        pimpl->swapProgram(initial);

        double startTime = glfwGetTime();
        st.startTime = startTime;

        // compute or fragment 감지
        bool hasCompute = false;
        auto detectCompute = [&](GLuint prog) {
            GLint count = 0;
            glGetProgramiv(prog, GL_ATTACHED_SHADERS, &count);
            std::vector<GLuint> arr(count);
            glGetAttachedShaders(prog, count, nullptr, arr.data());
            bool c = false;
            for (GLuint s : arr) {
                GLint type = 0;
                glGetShaderiv(s, GL_SHADER_TYPE, &type);
                if (type == GL_COMPUTE_SHADER) c = true;
            }
            return c;
        };
        hasCompute = detectCompute(initial);

        while (!glfwWindowShouldClose(st.window)) {

            auto now = fs::last_write_time(shaderPath);
            if (now != lastTime) {
                lastTime = now;
                std::cout << "[AutoGL] shader changed, recompiling\n";

                GLuint np = pimpl->tryLoadProgram(shaderPath);
                if (np != 0) {
                    pimpl->swapProgram(np);
                    hasCompute = detectCompute(np);
                    st.startTime = glfwGetTime();
                }
            }

            glClear(GL_COLOR_BUFFER_BIT);

            if (pimpl->currentProgram) {
                glUseProgram(pimpl->currentProgram);
                detail::setBuiltinUniforms(pimpl->currentProgram, st);

                if (hasCompute) {
                    glDispatchCompute(8, 8, 1);
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                } else {
                    glBindVertexArray(st.quadVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }

            glfwSwapBuffers(st.window);
            glfwPollEvents();

            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "GL error in mainLoop: " << err << "\n";
            }
        }
    }



}

