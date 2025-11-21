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
        if (sections.fragment.empty() && sections.compute.empty()) {
            AUTOGL_LOG_ERROR("GLSLLoader",
                             " fragment or compute shader is required");
            return 0;
        }

        GLuint vert = 0, frag = 0, comp = 0;

        if (!sections.vertex.empty()) {
            vert = GL::CompileVertexShader(sections.vertex);
            if (!vert) {
                return 0;
            }
        }
        if (!sections.fragment.empty()) {
            frag = GL::CompileFragmentShader(sections.fragment);
            if (!frag) {
                if (vert) glDeleteShader(vert);
                return 0;
            }
        }
        if (!sections.compute.empty()) {
            comp = GL::CompileComputeShader(sections.compute);
            if (!comp) {
                if (vert) glDeleteShader(vert);
                if (frag) glDeleteShader(frag);
                return 0;
            }
        }

        GLuint program = glCreateProgram();
        if (!program) {
            AUTOGL_LOG_ERROR("GLSLLoader", " glCreateProgram failed");
            if (vert) glDeleteShader(vert);
            if (frag) glDeleteShader(frag);
            if (comp) glDeleteShader(comp);
            return 0;
        }

        if (vert) glAttachShader(program, vert);
        if (frag) glAttachShader(program, frag);
        if (comp) glAttachShader(program, comp);

        glLinkProgram(program);

        GLint ok = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[2048];
            glGetProgramInfoLog(program, 2048, nullptr, log);
            AUTOGL_LOG_ERROR("GLSLLoader",
                             std::string(" program link failed: ") + log);

            glDeleteProgram(program);
            if (vert) glDeleteShader(vert);
            if (frag) glDeleteShader(frag);
            if (comp) glDeleteShader(comp);
            return 0;
        }

        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        if (comp) glDeleteShader(comp);

        // compute 세션이 있을 경우 SSBO auto binding
        if (!sections.compute.empty()) {
            auto bindings = ScanSsboBindings(sections.compute);

            // 기본 버퍼 크기: 4 KB. 나중에 compute layout 기반으로 늘릴 수 있음.
            constexpr std::size_t defaultSize = 4096;

            for (const auto& b : bindings) {
                if (b.binding < 0) continue;

                unsigned int ssbo = createEmptySSBO(defaultSize);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                                 static_cast<GLuint>(b.binding),
                                 ssbo);

                AUTOGL_LOG_INFO("GLSLLoader",
                    "SSBO created for binding "
                    + std::to_string(b.binding)
                    + " size " + std::to_string(defaultSize));
            }
        }

        return program;
    }

} // namespace AutoGL
