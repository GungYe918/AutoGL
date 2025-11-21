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

namespace AutoGL::detail {

    static unsigned int createTypedSSBO(std::size_t count, std::size_t elemSize) {
        GLuint ssbo = 0;
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, count * elemSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return ssbo;
    }

    static int getActiveSSBOCount(GLuint program) {
        GLint count = 0;
        glGetProgramInterfaceiv(
            program,
            GL_SHADER_STORAGE_BLOCK,
            GL_ACTIVE_RESOURCES,
            &count
        );
        return count;
    }

    static void reflectSSBOBindings(GLuint program, std::unordered_map<int, SSBOTypeInfo>& out) {
        const int count = getActiveSSBOCount(program);
        if (count <= 0) return;

        const GLenum props[1] = { GL_BUFFER_BINDING };

        for (int i = 0; i < count; i++) {
            // === Resource Name ===
            char nameBuf[256];
            GLsizei len = 0;
            glGetProgramResourceName(
                program,
                GL_SHADER_STORAGE_BLOCK,
                i,
                sizeof(nameBuf),
                &len,
                nameBuf
            );

            // === Binding Index ===
            GLint binding = -1;
            glGetProgramResourceiv(
                program,
                GL_SHADER_STORAGE_BLOCK,
                i, 1, props, 1, nullptr,
                &binding
            );

            if (binding < 0) continue;

            // 이름 기반으로 타입 추론 시도
            SSBOTypeInfo tinfo;
            auto it = out.find(binding);

            if (it != out.end()) {
                // 파서에서 이미 알아낸 타입 정보 존재
                tinfo = it->second;
            } else {
                // reflection만으로는 타입 알 수 없음 → 기본 float scalar
                tinfo = ParseSingleType("float");
            }

            // stride가 0이면 fallback
            if (tinfo.stride <= 0) tinfo.stride = 4;

            // 이미 생성된 SSBO가 있는지 검사
            GLint currentSSBO = 0;
            glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, binding, &currentSSBO);

            if (currentSSBO == 0) {
                GLuint ssbo = 0;
                glGenBuffers(1, &ssbo);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

                // 1024개의 요소를 담는 버퍼 생성
                std::size_t totalBytes = static_cast<std::size_t>(1024) * tinfo.stride;

                glBufferData(
                    GL_SHADER_STORAGE_BUFFER,
                    totalBytes,
                    nullptr,           // 초기 데이터 없음 (zero-initialized)
                    GL_DYNAMIC_DRAW
                );

                // 다시 바인딩 해제
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

                // binding 슬롯에 연결
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
            }

            // 저장
            out[binding] = tinfo;
        }
    }


} // namespace AutoGL::detail

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

    LoadedShaderProgram loadShaderProgram(const std::string& path) {
        LoadedShaderProgram result;

        std::string full = loadFileSource(path);
        if (full.empty()) {
            return result;
        }

        ShaderSourceSet sections = ExtractShaderSections(full);
        if (sections.fragment.empty() && sections.compute.empty() && sections.vertex.empty()) {
            AUTOGL_LOG_ERROR("GLSLLoader",
                " shader file must contain at least one @type");
            return result;
        }

        const bool hasVert    = !sections.vertex.empty();
        const bool hasFrag    = !sections.fragment.empty();
        const bool hasCompute = !sections.compute.empty();

        // compute + vertex/fragment 혼합 금지
        if (hasCompute && (hasVert || hasFrag)) {
            AUTOGL_LOG_ERROR("GLSLLoader",
                "@type compute cannot be mixed with vertex/fragment");
            return result;
        }

        GLuint program = glCreateProgram();
        if (!program) {
            AUTOGL_LOG_ERROR("GLSLLoader", "glCreateProgram failed");
            return result;
        }

        // ==========================================================
        // CASE 1: Compute-only
        // ==========================================================
        if (hasCompute && !hasVert && !hasFrag) {

            GLuint comp = GL::CompileComputeShader(sections.compute);
            if (!comp) {
                glDeleteProgram(program);
                return result;
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
                return result;
            }
            glDeleteShader(comp);

            // ============================
            // SSBO Binding 파싱
            // ============================
            auto bindings = ScanSsboBindings(sections.compute);

            for (auto& b : bindings) {
                if (b.binding < 0) continue;

                // (1) 타입 파싱
                SSBOTypeInfo tinfo = ParseSingleType(b.typeName);
                tinfo.isArray = b.isArray;

                // stride가 0이라면 fallback
                if (tinfo.stride <= 0) {
                    AUTOGL_LOG_WARN("GLSLLoader", "Type " + b.typeName +
                        " produced invalid stride; fallback to float");
                    tinfo = ParseSingleType("float");
                }

                // SSBO 크기 = 1024 * stride
                GLuint ssbo = AutoGL::detail::createTypedSSBO(1024, tinfo.stride);

                glBindBufferBase(
                    GL_SHADER_STORAGE_BUFFER,
                    static_cast<GLuint>(b.binding),
                    ssbo
                );

                // 저장: binding -> 완전 타입 정보
                result.bindingTypeInfo[b.binding] = tinfo;
            }

            AutoGL::detail::reflectSSBOBindings(program, result.bindingTypeInfo);

            AUTOGL_LOG_INFO("GLSLLoader", "Compute-only program built");
            result.program = program;
            return result;
        }

        // ==========================================================
        // CASE 2: Graphics shader (vertex + fragment)
        // ==========================================================
        GLuint vert = 0, frag = 0;

        if (hasVert) {
            vert = GL::CompileVertexShader(sections.vertex);
            if (!vert) {
                glDeleteProgram(program);
                return result;
            }
            glAttachShader(program, vert);
        }

        if (hasFrag) {
            frag = GL::CompileFragmentShader(sections.fragment);
            if (!frag) {
                if (vert) glDeleteShader(vert);
                glDeleteProgram(program);
                return result;
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
            return result;
        }

        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);

        AUTOGL_LOG_INFO("GLSLLoader", "Graphic program built");

        result.program = program;
        return result;
    }

    
} // namespace AutoGL
