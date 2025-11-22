// panels/PanelManager.hpp
#pragma once
#include <unordered_map>
#include <memory>
#include "Panel.hpp"

class PanelManager {
public:
    void registerPanel(std::shared_ptr<Panel> panel) {
        panels[panel->name()] = panel;
    }

    std::string dispatch(const std::string& panel, const std::string& msg) {
        if (panels.count(panel)) {
            return panels[panel]->onMessage(msg);
        }
        return "{}";
    }

    void initAll() {
        for (auto& p : panels) {
            p.second->onLoad();
        }
    }

    
private:
    std::unordered_map<std::string, std::shared_ptr<Panel>> panels;
};