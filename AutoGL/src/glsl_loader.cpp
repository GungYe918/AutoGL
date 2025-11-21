// src/glsl_loader.cpp
#include "glsl_loader.hpp"

#include "shader_regex.hpp"
#include "gl_shader_vertex.hpp"
#include "gl_shader_fragment.hpp"
#include "gl_shader_compute.hpp"

#include <AutoGL/Log.hpp>

#include <fstream>
#include <sstream>
#include <vector>

namespace AutoGL {

    std::string loadFileSource(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            AUTOGL_LOG_ERROR("GLSLLoader", " failed to open file " + path);
            return {};
        }

        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    // 기존 API 호환용
    GLuint compileShader(GLenum type, const std::string& source) {
        using namespace GL;
        switch (type) {
            case GL_VERTEX_SHADER:   return CompileVertexShader(source);
            case GL_FRAGMENT_SHADER: return CompileFragmentShader(source);
            case GL_COMPUTE_SHADER:  return CompileComputeShader(source);
            default:
                AUTOGL_LOG_ERROR("GLSLLoader", " unsupported shader type in compileShader");
                return 0;
        }
    }

    // compute용 SSBO auto 생성
    static unsigned int createEmptySSBO(std::size_t size) {
        unsigned int ssbo = 0;
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return ssbo;
    }

    GLuint loadShaderProgram(const std::string& path) {
        std::string full = loadFileSource(path);
        if (full.empty()) {
            return 0;
        }

        ShaderSourceSet sections = ExtractShaderSections(full);
        if (sections.fragment.empty() && sections.compute.empty() && sections.vertex.empty()) {
            AUTOGL_LOG_ERROR("GLSLLoader",
                " shader file must contain at least one @type");
            return 0;
        }

        const bool hasVert    = !sections.vertex.empty();
        const bool hasFrag    = !sections.fragment.empty();
        const bool hasCompute = !sections.compute.empty();

        // ----- NEW: compute + vertex/fragment 섞이면 에러 -----
        if (hasCompute && (hasVert || hasFrag)) {
            AUTOGL_LOG_ERROR(
                "GLSLLoader",
                " @type compute cannot be mixed with vertex/fragment in the same file"
            );
            return 0;
        }

        GLuint program = glCreateProgram();
        if (!program) {
            AUTOGL_LOG_ERROR("GLSLLoader", " glCreateProgram failed");
            return 0;
        }

        // ---------------------------
        // CASE 1: 순수 Compute 전용 모드
        //   - compute만 있고, vertex/fragment는 없음
        // ---------------------------
        if (hasCompute && !hasVert && !hasFrag) {

            GLuint comp = GL::CompileComputeShader(sections.compute);
            if (!comp) {
                glDeleteProgram(program);
                return 0;
            }

            glAttachShader(program, comp);
            glLinkProgram(program);

            GLint ok = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &ok);
            if (!ok) {
                char log[2048];
                glGetProgramInfoLog(program, 2048, nullptr, log);
                AUTOGL_LOG_ERROR("ComputeLink", log);

                glDeleteShader(comp);
                glDeleteProgram(program);
                return 0;
            }

            glDeleteShader(comp);

            // SSBO auto binding
            auto bindings = ScanSsboBindings(sections.compute);
            constexpr std::size_t defaultSize = 4096;

            for (auto& b : bindings) {
                if (b.binding < 0) continue;
                unsigned int ssbo = createEmptySSBO(defaultSize);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                                static_cast<GLuint>(b.binding),
                                ssbo);
            }

            AUTOGL_LOG_INFO("GLSLLoader", "Compute-only program built");
            return program;
        }

        // ---------------------------
        // CASE 2: 그래픽 (Vertex + Fragment) 프로그램
        // ---------------------------
        GLuint vert = 0, frag = 0;

        if (hasVert) {
            vert = GL::CompileVertexShader(sections.vertex);
            if (!vert) {
                glDeleteProgram(program);
                return 0;
            }
            glAttachShader(program, vert);
        }

        if (hasFrag) {
            frag = GL::CompileFragmentShader(sections.fragment);
            if (!frag) {
                if (vert) glDeleteShader(vert);
                glDeleteProgram(program);
                return 0;
            }
            glAttachShader(program, frag);
        }

        glLinkProgram(program);

        GLint ok = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[2048];
            glGetProgramInfoLog(program, 2048, nullptr, log);
            AUTOGL_LOG_ERROR("GraphicLink", log);

            glDeleteProgram(program);
            if (vert) glDeleteShader(vert);
            if (frag) glDeleteShader(frag);
            return 0;
        }

        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);

        AUTOGL_LOG_INFO("GLSLLoader", "Graphic program built");

        return program;
    }

} // namespace AutoGL
