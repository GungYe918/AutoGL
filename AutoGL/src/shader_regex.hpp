// src/shader_regex.hpp
#pragma once
#include <string>
#include <vector>
#include <cstddef>

namespace AutoGL {

    struct ShaderSourceSet {
        std::string vertex;
        std::string fragment;
        std::string compute;
    };

    struct SsboBinding {
        int binding = -1;

        std::string typeName;   // float / vec4 / FooStruct
        std::string varName;    // 변수 이름  (acc, data 등)

        bool isArray = false;   // acc[] 같은 경우 true

        // TODO: SSBOTypeInfo ParseSingleType와 매칭
    };

    struct ComputeLayoutInfo {
        int localSizeX = 1;
        int localSizeY = 1;
        int localSizeZ = 1;
    };

    struct ShaderError {
        int line = -1;          // -1이면 정보 없음
        std::string message;
    };

    // "@type vertex", "@type fragment", "@type compute" 섹션 분리
    ShaderSourceSet ExtractShaderSections(const std::string& fullSource);

    // layout(binding = N) buffer ... 를 전부 검색
    std::vector<SsboBinding> ScanSsboBindings(const std::string& source);

    // layout(local_size_x = a, local_size_y = b, local_size_z = c) 파싱
    ComputeLayoutInfo ParseComputeLayout(const std::string& source);

    // GLSL 에러 로그에서 "file:line:" 형태의 라인 정보를 대충 추출
    std::vector<ShaderError> ParseGlslErrorLog(const std::string& log);
} // namespace AutoGL
