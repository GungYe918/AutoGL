// panels/TreeView.hpp
#pragma once
#include "Panel.hpp"
#include <filesystem>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <algorithm>

#include <nlohmann/json.hpp>


class TreeViewPanel : public Panel {
public:
    std::string name() const override {
        return "treeview";
    }

    std::string onMessage(const std::string& msg) override {
        using json = nlohmann::json;

        json req = json::parse(msg);
        std::string action = req["action"];

        json resp;

        if (action == "list_root") {
            auto root = scanDir(std::filesystem::current_path());
            resp["action"] = "root";
            resp["tree"] = root;
            return resp.dump();
        }

        else if (action == "list_child") {
            std::string folder = req["folder"];
            auto child = scanDir(std::filesystem::current_path() / folder);
            resp["action"] = "child";
            resp["tree"] = child;
            return resp.dump();

        } 
        
        else if (action == "read_file") {
            std::string file = req.value("file", "");

            nlohmann::json resp;
            resp["action"] = "file";
            resp["path"]   = file;

            // 안전 가드
            if (file.empty()) {
                resp["content"] = "// Error: empty file path\n";
                return resp.dump();
            }

            std::filesystem::path fullPath =
                std::filesystem::current_path() / file;

            if (!std::filesystem::exists(fullPath) ||
                !std::filesystem::is_regular_file(fullPath)) {
                resp["content"] = "// Error: File not found\n";
                return resp.dump();
            }

            // --- 1) 확장자 기반 바이너리 필터링 ---
            auto ext = fullPath.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

            static const std::unordered_set<std::string> kBinaryExts = {
                ".o", ".a", ".so", ".dll", ".dylib", ".exe",
                ".png", ".jpg", ".jpeg", ".gif", ".bmp",
                ".ico", ".ttf", ".otf", ".woff", ".woff2",
                ".zip", ".tar", ".gz", ".7z"
            };

            if (kBinaryExts.count(ext) > 0) {
                resp["binary"]  = true;
                resp["content"] =
                    "// Binary file: viewing is not supported in this editor.\n";
                return resp.dump();
            }

            // --- 2) 내용 읽기 (텍스트 가정) ---
            std::ifstream ifs(fullPath, std::ios::binary);
            if (!ifs) {
                resp["content"] = "// Error: Failed to open file\n";
                return resp.dump();
            }

            std::string content((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());

            // NUL 바이트가 있으면 바이너리로 간주 (보수적 방어)
            if (content.find('\0') != std::string::npos) {
                resp["binary"]  = true;
                resp["content"] =
                    "// Binary or non-text file (contains NUL bytes). Not shown.\n";
                return resp.dump();
            }

            resp["binary"]  = false;
            resp["content"] = content;

            // --- 3) UTF-8 문제 대비: dump() 예외 방어 ---
            try {
                return resp.dump();
            } catch (const nlohmann::json::type_error& e) {
                nlohmann::json safe;
                safe["action"]  = "file";
                safe["path"]    = file;
                safe["binary"]  = true;
                safe["content"] =
                    "// Error: file is not valid UTF-8 (probably binary). "
                    "Cannot display in editor.\n";
                return safe.dump();
            }
        }
        else if(action == "save_file") {
            std::string file = req["file"];
            std::string content = req["content"];

            std::filesystem::path fullPath =
                std::filesystem::current_path() / file;

            std::ofstream ofs(fullPath);
            ofs << content;
            ofs.close();

            json resp;
            resp["action"] = "saved";
            resp["file"] = file;
            return resp.dump();
        }


        resp["error"] = "unknown action";
        return resp.dump();
    }

private:
    nlohmann::json scanDir(const std::filesystem::path& path) {
        using json = nlohmann::json;
        json result;

        if (!std::filesystem::exists(path))
            return result;

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::string name = entry.path().filename().string();

            if (entry.is_directory()) {
                result[name] = scanDir(entry.path());
            } else {
                result[name] = true;
            }
        }

        return result;
    }
};