// gui/panels/Sidebar.hpp
#pragma once
#include "Panel.hpp"

class Sidebar : public Panel {
public:
    std::string name() const override {
        return "sidebar";
    }

    std::string onMessage(const std::string& msg) override {
        return R"({"result":"ok"})";
    }

    void onLoad() override {
        // 초기화 시 JS로 업데이트 보내기 가능
    }
};