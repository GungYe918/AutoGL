// src/engine_gl.cpp
#include "gl_engine.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glsl_loader.hpp"
#include "shader_regex.hpp"
#include <AutoGL/Log.hpp>

#include <filesystem>
#include <iostream>
#include <unordered_map>

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

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/) {
        auto* st = static_cast<InternalGLState*>(glfwGetWindowUserPointer(window));
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

    void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
        auto* st = static_cast<InternalGLState*>(glfwGetWindowUserPointer(window));
        if (!st) return;

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);

        st->mouseX = xpos;
        st->mouseY = h - ypos;
    }

    void setBuiltinUniforms(unsigned int program, InternalGLState& st) {
        // 시간
        float timeNow = static_cast<float>(glfwGetTime() - st.startTime);

        GLint uTime = glGetUniformLocation(program, "iTime");
        if (uTime >= 0) {
            glUniform1f(uTime, timeNow);
        }

        double now = glfwGetTime();
        st.deltaTime = now - st.prevFrameTime;
        st.prevFrameTime = now;

        GLint uTimeDelta = glGetUniformLocation(program, "iTimeDelta");
        if (uTimeDelta >= 0) {
            glUniform1f(uTimeDelta, static_cast<float>(st.deltaTime));
        }

        // frame
        st.frameCount++;
        GLint uFrame = glGetUniformLocation(program, "iFrame");
        if (uFrame >= 0) {
            glUniform1i(uFrame, st.frameCount);
        }

        // GlobalTime alias
        GLint uGlobalTime = glGetUniformLocation(program, "iGlobalTime");
        if (uGlobalTime >= 0) {
            glUniform1f(uGlobalTime, timeNow);
        }

        // resolution
        int w, h;
        glfwGetFramebufferSize(st.window, &w, &h);
        GLint uRes = glGetUniformLocation(program, "iResolution");
        if (uRes >= 0) {
            glUniform2f(uRes, static_cast<float>(w), static_cast<float>(h));
        }

        // mouse
        GLint uMouse = glGetUniformLocation(program, "iMouse");
        if (uMouse >= 0) {
            glUniform4f(
                uMouse,
                static_cast<float>(st.mouseX),
                static_cast<float>(st.mouseY),
                st.mouseDown ? static_cast<float>(st.clickX) : 0.0f,
                st.mouseDown ? static_cast<float>(st.clickY) : 0.0f
            );
        }

        // date
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
                    static_cast<float>(lt->tm_year + 1900),
                    static_cast<float>(lt->tm_mon + 1),
                    static_cast<float>(lt->tm_mday),
                    seconds
                );
            }
        }

        // frame rate
        float frameRate = (st.deltaTime > 0.0)
            ? static_cast<float>(1.0 / st.deltaTime)
            : 0.0f;

        GLint uRate = glGetUniformLocation(program, "iFrameRate");
        if (uRate >= 0) {
            glUniform1f(uRate, frameRate);
        }

        // random
        st.randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        GLint uRand = glGetUniformLocation(program, "iRandom");
        if (uRand >= 0) {
            glUniform1f(uRand, st.randomValue);
        }

        // iChannelResolution, iChannelTime
        for (int i = 0; i < 4; ++i) {
            std::string name = "iChannelResolution[" + std::to_string(i) + "]";
            GLint loc = glGetUniformLocation(program, name.c_str());
            if (loc >= 0) {
                glUniform3f(loc,
                    static_cast<float>(st.texWidth[i]),
                    static_cast<float>(st.texHeight[i]),
                    1.0f);
            }

            std::string tname = "iChannelTime[" + std::to_string(i) + "]";
            GLint tloc = glGetUniformLocation(program, tname.c_str());
            if (tloc >= 0) {
                glUniform1f(tloc,
                    static_cast<float>(glfwGetTime() - st.channelTime[i]));
            }
        }

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            AUTOGL_LOG_ERROR("EngineGL",
                "GL error in setBuiltinUniforms code " + std::to_string(err));
        }
    }

    
    bool hasComputeStage(unsigned int program) {
        GLint numShaders = 0;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &numShaders);
        if (numShaders <= 0) return false;

        std::vector<GLuint> attached(static_cast<std::size_t>(numShaders));
        glGetAttachedShaders(program, numShaders, nullptr, attached.data());

        bool hasCompute   = false;
        bool hasNonCompute = false;

        for (GLuint s : attached) {
            GLint type = 0;
            glGetShaderiv(s, GL_SHADER_TYPE, &type);
            if (type == GL_COMPUTE_SHADER) {
                hasCompute = true;
            } else {
                hasNonCompute = true;
            }
        }

        // “compute-only 프로그램인지” 여부 반환
        return hasCompute && !hasNonCompute;
    }

    static bool SafeDispatchCompute(GLuint program, int dummyX, int dummyY, int dummyZ) {
        // 1) 먼저 local_size_x/y/z 파악
        GLint computeShader = 0;
        GLint numShaders = 0;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &numShaders);

        std::vector<GLuint> attached(numShaders);
        glGetAttachedShaders(program, numShaders, nullptr, attached.data());

        std::string computeSource;
        for (auto s : attached) {
            GLint type = 0;
            glGetShaderiv(s, GL_SHADER_TYPE, &type);
            if (type == GL_COMPUTE_SHADER) {
                // compute shader 원본 가져오기
                GLint len = 0;
                glGetShaderiv(s, GL_SHADER_SOURCE_LENGTH, &len);
                if (len > 0) {
                    computeSource.resize(len);
                    glGetShaderSource(s, len, nullptr, computeSource.data());
                }
                computeShader = s;
                break;
            }
        }

        // 기본값
        int localX = 1, localY = 1, localZ = 1;

        if (!computeSource.empty()) {
            auto info = AutoGL::ParseComputeLayout(computeSource);
            localX = info.localSizeX;
            localY = info.localSizeY;
            localZ = info.localSizeZ;
        }

        // 작업량 자동 계산 (기본 1024 항목 기준)
        const int TOTAL = 1024;
        int dispatchX = (TOTAL + localX - 1) / localX;
        int dispatchY = (1 + localY - 1) / localY;
        int dispatchZ = (1 + localZ - 1) / localZ;

        // 기존 GL 에러 플러시
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {}

        glDispatchCompute(dispatchX, dispatchY, dispatchZ);

        err = glGetError();
        if (err != GL_NO_ERROR) {
            AUTOGL_LOG_FATAL("Compute", "DispatchCompute failed: error " + std::to_string(err));
            return false;
        }

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        err = glGetError();
        if (err != GL_NO_ERROR) {
            AUTOGL_LOG_FATAL("Compute", "MemoryBarrier failed: error " + std::to_string(err));
            return false;
        }

        return true;
    }

    static constexpr uint32_t SSBO_INIT_PATTERN = 0xCDCDCDCD;

    void DumpAllSSBOs(const std::unordered_map<int, SSBOTypeInfo>& bindingInfos) {
        GLint maxBindings = 0;
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxBindings);

        const int RESERVED = 4;
        const int BASE_SHOW_COUNT = 16;
        bool printedAny = false;

        for (int binding = 0; binding < maxBindings; binding++) {

            GLint ssbo = 0;
            glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, binding, &ssbo);

            if (ssbo == 0) {
                if (binding < RESERVED)
                    AUTOGL_LOG_INFO("SSBO", "[#" + std::to_string(binding) + "] NONE");
                continue;
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

            GLint size = 0;
            glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &size);
            if (size <= 0) {
                AUTOGL_LOG_INFO("SSBO", "[#" + std::to_string(binding) + "] NONE");
                continue;
            }

            void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            if (!ptr) {
                AUTOGL_LOG_ERROR("SSBO", "Failed to map SSBO #" + std::to_string(binding));
                continue;
            }

            SSBOTypeInfo info;
            auto it = bindingInfos.find(binding);
            if (it != bindingInfos.end())
                info = it->second;
            else
                info = ParseSingleType("float");

            const uint8_t* base = reinterpret_cast<const uint8_t*>(ptr);
            int stride = info.stride;
            int count = size / stride;

            std::vector<int> meaningful;
            std::vector<int> realZero;
            std::vector<int> uninitialized;

            for (int i = 0; i < count; i++) {
                const uint8_t* p = base + i * stride;

                if (info.isScalar()) {
                    uint32_t v = *reinterpret_cast<const uint32_t*>(p);

                    if (v == SSBO_INIT_PATTERN) {
                        uninitialized.push_back(i);
                    }
                    else if (v == 0) {
                        realZero.push_back(i);
                    }
                    else {
                        meaningful.push_back(i);
                    }
                }
                else {
                    // 구조체나 벡터의 경우 최소 한 요소만 체크
                    uint32_t first = *reinterpret_cast<const uint32_t*>(p);
                    if (first == SSBO_INIT_PATTERN) {
                        uninitialized.push_back(i);
                    }
                    else {
                        bool allZero = true;
                        for (int b = 0; b < stride; b++) {
                            if (p[b] != 0) { allZero = false; break; }
                        }
                        if (allZero) realZero.push_back(i);
                        else meaningful.push_back(i);
                    }
                }
            }

            // 모든 값이 INIT 패턴
            if (!meaningful.empty() || !realZero.empty())
            {
                int total = meaningful.size() + realZero.size();
                int limit = std::min<int>(BASE_SHOW_COUNT, total);

                AUTOGL_LOG_INFO(
                    "SSBO",
                    "[#" + std::to_string(binding) + "] showing "
                        + std::to_string(limit)
                        + " / " + std::to_string(total)
                        + " items"
                );

                auto printScalar = [&](int idx, const char* tag) {
                    const uint8_t* p = base + idx * stride;
                    float v = *reinterpret_cast<const float*>(p);
                    AUTOGL_LOG_INFO("SSBO",
                        "[" + std::to_string(idx) + "] " + tag + " = " + std::to_string(v));
                };

                int printed = 0;

                // 1) meaningful 값 출력
                for (int idx : meaningful) {
                    printScalar(idx, "VAL");
                    if (++printed >= limit) break;
                }
                if (printed < limit) {
                    // 2) realZero 값 출력
                    for (int idx : realZero) {
                        printScalar(idx, "ZERO");
                        if (++printed >= limit) break;
                    }
                }

                if (meaningful.size() + realZero.size() > BASE_SHOW_COUNT) {
                    AUTOGL_LOG_INFO("SSBO", "... more meaningful or zero values exist");
                }

                printedAny = true;
            }
            else {
                AUTOGL_LOG_INFO("SSBO",
                    "[#" + std::to_string(binding) + "] UNINITIALIZED (all pattern 0xCDCDCDCD)");
            }

            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }

        if (!printedAny)
            AUTOGL_LOG_INFO("SSBO", "No SSBO found");
    }

} // AutoGL::detail


namespace AutoGL {

    
    EngineGLBackend::EngineGLBackend() = default;

    EngineGLBackend::~EngineGLBackend() {
        if (currentProgram_ != 0) {
            glDeleteProgram(currentProgram_);
            currentProgram_ = 0;
        }
        if (state_.quadVBO) {
            glDeleteBuffers(1, &state_.quadVBO);
            state_.quadVBO = 0;
        }
        if (state_.quadVAO) {
            glDeleteVertexArrays(1, &state_.quadVAO);
            state_.quadVAO = 0;
        }
        if (state_.window) {
            glfwDestroyWindow(state_.window);
            state_.window = nullptr;
            glfwTerminate();
        }
    }

    bool EngineGLBackend::initContext() {
        if (!glfwInit()) {
            AUTOGL_LOG_FATAL("EngineGL", "GLFW init failed");
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        state_.window = glfwCreateWindow(
            state_.width, state_.height, "AutoGL Engine", nullptr, nullptr);
        if (!state_.window) {
            AUTOGL_LOG_FATAL("EngineGL", "GLFW window creation failed");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(state_.window);
        glfwSetWindowUserPointer(state_.window, &state_);

        glfwSetCursorPosCallback(state_.window, AutoGL::detail::cursor_pos_callback);
        glfwSetMouseButtonCallback(state_.window, AutoGL::detail::mouse_button_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            AUTOGL_LOG_FATAL("EngineGL", "GLAD load failed");
            return false;
        }

        detail::createFullscreenQuad(state_);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        state_.startTime     = glfwGetTime();
        state_.prevFrameTime = state_.startTime;
        state_.frameCount    = 0;

        return true;
    }

    bool EngineGLBackend::init() {
        return initContext();
    }

    void EngineGLBackend::setWindowSize(int w, int h) {
        state_.width  = w;
        state_.height = h;

        if (state_.window) {
            glfwSetWindowSize(state_.window, w, h);
        }
    }

    unsigned int EngineGLBackend::tryLoadProgram(const std::string& path) {
        LoadedShaderProgram ls = loadShaderProgram(path);
        if (ls.program == 0) {
            AUTOGL_LOG_ERROR("EngineGL", "shader compile failed, keep old program");
            return 0;  
        } 
        return ls.program;
    }

    void EngineGLBackend::swapProgram(unsigned int newProgram) {
        if (!newProgram) return;

        if (currentProgram_ != 0) {
            glDeleteProgram(currentProgram_);
        }
        currentProgram_ = newProgram;
        glUseProgram(currentProgram_);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            AUTOGL_LOG_ERROR("EngineGL",
                "GL error after using new program code " + std::to_string(err));
        }
    }

    bool EngineGLBackend::runShaderFile(const std::string& path) {
        LoadedShaderProgram ls = loadShaderProgram(path);
        GLuint program = ls.program;

        if (program == 0)
            return false;

        isComputeMode_ = AutoGL::detail::hasComputeStage(program);
        glUseProgram(program);

        const bool isCompute = AutoGL::detail::hasComputeStage(program);

        state_.startTime     = glfwGetTime();
        state_.prevFrameTime = state_.startTime;
        state_.frameCount    = 0;

        // ========================================================
        // Compute 전용 모드
        // ========================================================
        if (isCompute) {
            AutoGL::detail::setBuiltinUniforms(program, state_);

            double t0 = glfwGetTime();
            bool ok   = AutoGL::detail::SafeDispatchCompute(program, 1, 1, 1);
            double t1 = glfwGetTime();

            if (!ok || (t1 - t0) > 0.5) {
                AUTOGL_LOG_FATAL("Compute",
                    "Compute dispatch failed or took too long.");
                glDeleteProgram(program);
                return false;
            }

            // 새로운 타입 기반 SSBO 덤프
            AutoGL::detail::DumpAllSSBOs(ls.bindingTypeInfo);

            glDeleteProgram(program);
            return true;
        }

        // ========================================================
        // Graphics 모드
        // ========================================================
        {
            int w, h;
            glfwGetFramebufferSize(state_.window, &w, &h);
            glViewport(0, 0, w, h);

            glClear(GL_COLOR_BUFFER_BIT);
            glBindVertexArray(state_.quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glfwSwapBuffers(state_.window);

            glDeleteProgram(program);
            return true;
        }
    }


    void EngineGLBackend::mainLoop(const std::string& shaderPath) {
        // 먼저 파일을 읽어서 어떤 타입인지 판단
        std::string fullSource = loadFileSource(shaderPath);
        ShaderSourceSet sections = ExtractShaderSections(fullSource);

        bool hasCompute = !sections.compute.empty();
        bool hasVert    = !sections.vertex.empty();
        bool hasFrag    = !sections.fragment.empty();

        // compute + vertex/fragment 섞이면 금지 (로더와 일관성)
        if (hasCompute && (hasVert || hasFrag)) {
            AUTOGL_LOG_ERROR(
                "EngineGL",
                "@type compute cannot be mixed with vertex/fragment in the same file"
            );
            return;
        }

        // ===== CASE 1: compute-only 파일이면, runShaderFile 한 번 실행 후 종료 =====
        if (hasCompute) {
            AUTOGL_LOG_INFO("EngineGL",
                "Compute-only shader: single dispatch then exit");
            runShaderFile(shaderPath);
            return;
        }

        // ===== CASE 2: 그래픽 전용 파일 (vertex/fragment만 있는 경우) =====
        fs::file_time_type lastTime = fs::last_write_time(shaderPath);

        GLuint initial = tryLoadProgram(shaderPath);
        swapProgram(initial);

        state_.startTime     = glfwGetTime();
        state_.prevFrameTime = state_.startTime;
        state_.frameCount    = 0;

        while (!glfwWindowShouldClose(state_.window)) {
            // hot reload
            auto now = fs::last_write_time(shaderPath);
            if (now != lastTime) {
                lastTime = now;
                AUTOGL_LOG_INFO("EngineGL", "shader changed, recompiling");

                GLuint np = tryLoadProgram(shaderPath);
                if (np != 0) {
                    swapProgram(np);
                    state_.startTime     = glfwGetTime();
                    state_.prevFrameTime = state_.startTime;
                    state_.frameCount    = 0;
                }
            }

            glClear(GL_COLOR_BUFFER_BIT);

            if (currentProgram_ != 0) {
                glUseProgram(currentProgram_);
                detail::setBuiltinUniforms(currentProgram_, state_);

                int w, h;
                glfwGetFramebufferSize(state_.window, &w, &h);
                glViewport(0, 0, w, h);

                glBindVertexArray(state_.quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glfwSwapBuffers(state_.window);
            glfwPollEvents();

            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR) {
                AUTOGL_LOG_ERROR("EngineGL",
                    "GL error in mainLoop code " + std::to_string(err));
            }
        }
    }

} // namespace AutoGL
