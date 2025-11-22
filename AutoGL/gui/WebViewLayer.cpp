// gui/WebViewLayer.cpp
#include "WebViewLayer.hpp"
#include <nlohmann/json.hpp>

#include <webview/webview.h>
#include "panels/Sidebar.hpp"
#include "panels/TreeView.hpp"

#if defined(__linux__)
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#endif

using json = nlohmann::json;

void WebViewLayer::run() {
    webview::webview w(true, nullptr);
    w.set_title("AutoGL IDE");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

    // 패널 등록
    panelManager.registerPanel(std::make_shared<Sidebar>());
    panelManager.registerPanel(std::make_shared<TreeViewPanel>());

    // JS -> C++
    w.bind("panelMessage", [&](std::string data) {
        json j = json::parse(data);

        // 예: ["{\"panel\":\"treeview\", \"body\":{...}}"]
        if (j.is_array() && j.size() == 1 && j[0].is_string()) {
            j = json::parse(j[0].get<std::string>());
        }

        // JSON에서 panel/body 추출
        std::string panel = j["panel"];
        std::string body  = j["body"];

        // 패널로 메시지 전달
        std::string resp = panelManager.dispatch(panel, body);

        // C++ → JS 메시지 전달
        std::string jsCall =
            "window.handlePanelMessage("
            "'" + panel + "',"
            + resp +
            ");";

        w.eval(jsCall);

        return std::string("{}");
    });


    // index.html 로드
    std::string base = std::filesystem::current_path().string();
    w.navigate("file://" + base + "/../gui/ui/index.html");

#if defined(__linux__)
    {
    // webview에서 GtkWindow 포인터 가져오기
    void* raw = w.window().value();
    GtkWidget* gtk_window = GTK_WIDGET(raw);

    GtkWidget* gtk_webview = gtk_bin_get_child(GTK_BIN(gtk_window));

    WebKitWebView* view = WEBKIT_WEB_VIEW(gtk_webview);
    WebKitSettings* settings = webkit_web_view_get_settings(view);

    // GPU / WebGL / Canvas acceleration ON
    webkit_settings_set_enable_webgl(settings, TRUE);
    webkit_settings_set_enable_accelerated_2d_canvas(settings, TRUE);
    webkit_settings_set_enable_developer_extras(settings, TRUE);
    webkit_settings_set_hardware_acceleration_policy(
        settings,
        WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS
    );

    g_object_set(G_OBJECT(settings),
        "enable-webaudio", TRUE,
        "enable-html5-local-storage", TRUE,
        "enable-smooth-scrolling", TRUE,
        NULL
    );

    webkit_web_view_set_settings(view, settings);
    }
#elif defined(__APPLE__)
    w.eval("console.log('WKWebView GPU acceleration active');");
#elif defined(_WIN32)
    w.eval("console.log('WebView2 GPU acceleration active');");
#endif

    // C++ -> JS 초기화 메시지
    w.init(R"(
        console.log("[JS] AutoGL IDE starting...");

        // JS -> C++ 메시지 송신 함수
        window.sendToPanel = function(panel, body) {
            const msg = JSON.stringify({
                panel: panel,
                body: body
            });
            return window.panelMessage(msg);
        };

        // C++ -> JS 메시지 수신 함수
        if (!window.handlePanelMessage) {
            window.handlePanelMessage = function(panel, data) {
                console.warn("[JS] No handler for panel:", panel, data);
            };
        }
    )");

    w.run();
}
