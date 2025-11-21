// src/shader_regex.cpp
#include "shader_regex.hpp"
#include <regex>
#include <iostream>

namespace AutoGL {

    ShaderSourceSet ExtractShaderSections(const std::string& fullSource) {
        ShaderSourceSet out;

        const std::string token = "@type ";
        std::vector<std::size_t> markers;
        std::size_t pos = fullSource.find(token);
        while (pos != std::string::npos) {
            markers.push_back(pos);
            pos = fullSource.find(token, pos + token.size());
        }

        if (markers.empty()) {
            return out;
        }

        markers.push_back(fullSource.size());

        for (std::size_t i = 0; i + 1 < markers.size(); ++i) {
            std::size_t start = markers[i];
            std::size_t next  = markers[i + 1];

            std::size_t lineEnd = fullSource.find('\n', start);
            if (lineEnd == std::string::npos || lineEnd > next) {
                lineEnd = next;
            }

            std::string header = fullSource.substr(start, lineEnd - start);
            std::string body   = fullSource.substr(lineEnd, next - lineEnd);

            if (header.find("@type vertex") != std::string::npos) {
                out.vertex = body;
            } else if (header.find("@type fragment") != std::string::npos) {
                out.fragment = body;
            } else if (header.find("@type compute") != std::string::npos) {
                out.compute = body;
            }
        }

        return out;
    }

    std::vector<SsboBinding> ScanSsboBindings(const std::string& source) {
        std::vector<SsboBinding> result;

        std::regex r("layout\\s*\\([^\\)]*binding\\s*=\\s*(\\d+)\\s*[^\\)]*\\)\\s*buffer");
        std::smatch m;

        std::string s = source;
        while (std::regex_search(s, m, r)) {
            int b = std::stoi(m[1].str());
            result.push_back(SsboBinding{b});
            s = m.suffix();
        }
        return result;
    }

    ComputeLayoutInfo ParseComputeLayout(const std::string& source) {
        ComputeLayoutInfo info{1, 1, 1};

        std::regex rx("local_size_x\\s*=\\s*(\\d+)");
        std::regex ry("local_size_y\\s*=\\s*(\\d+)");
        std::regex rz("local_size_z\\s*=\\s*(\\d+)");

        std::smatch m;

        if (std::regex_search(source, m, rx)) {
            info.localSizeX = std::stoi(m[1].str());
        }
        if (std::regex_search(source, m, ry)) {
            info.localSizeY = std::stoi(m[1].str());
        }
        if (std::regex_search(source, m, rz)) {
            info.localSizeZ = std::stoi(m[1].str());
        }

        return info;
    }

    std::vector<ShaderError> ParseGlslErrorLog(const std::string& log) {
        std::vector<ShaderError> errors;

        // most drivers use "0(line)" or "0:line:" or "(line)" 같은 형태
        std::regex r_line(R"((\d+)[:\(](\d+)[\):])");
        std::smatch m;

        std::string s = log;
        while (std::regex_search(s, m, r_line)) {
            // m[2]가 실제 line일 가능성이 높음
            int line = -1;
            try {
                line = std::stoi(m[2].str());
            } catch (...) {
                line = -1;
            }

            ShaderError e;
            e.line = line;
            // 현재로서는 전체 로그를 그대로 메시지로 저장
            e.message = log;
            errors.push_back(e);

            s = m.suffix();
        }

        if (errors.empty() && !log.empty()) {
            ShaderError e;
            e.line = -1;
            e.message = log;
            errors.push_back(e);
        }

        return errors;
    }

} // namespace AutoGL
