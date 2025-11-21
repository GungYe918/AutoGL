// gui/WebViewLayer.cpp
#include "WebViewLayer.hpp"
#include <nlohmann/json.hpp>

#include <webview/webview.h>
#include "panels/Sidebar.hpp"

using json = nlohmann::json;

void WebViewLayer::run() {
    webview::webview w(true, nullptr);
    w.set_title("AutoGL IDE");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

    // 패널 등록
    panelManager.registerPanel(std::make_shared<Sidebar>());
    //panelManager.registerPanel(std::make_shared<ConsolePanel>());
    //panelManager.registerPanel(std::make_shared<EditorPanel>());

    // JS -> C++
    w.bind("panelMessage", [&](std::string data) {
        auto j = json::parse(data);
        std::string panel = j["panel"];
        std::string body  = j["body"];
        
        std::string resp = panelManager.dispatch(panel, body);
        return resp;
    });

    std::string base = std::filesystem::current_path().string();
    w.navigate("file://" + base + "/../gui/ui/index.html");

    // C++ -> JS 초기화
    w.init(R"(
        console.log("[JS] AutoGL IDE starting...");
    )");

    w.run();


    
}
