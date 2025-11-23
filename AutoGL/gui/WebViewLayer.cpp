// gui/WebViewLayer.cpp
#include "WebViewLayer.hpp"
#include <nlohmann/json.hpp>

#include <webview/webview.h>
#include "panels/Sidebar.hpp"
#include "panels/TreeView.hpp"
#include "panels/TerminalPanel.hpp"

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

    auto term = std::make_shared<TerminalPanel>();
    term->webviewEval = [&](const std::string& js) {
        w.eval(js);
    };
    panelManager.registerPanel(term);

    // JS -> C++
    w.bind("panelMessage", [&](std::string data) -> std::string {
        json j = json::parse(data);

        // 예: ["{\"panel\":\"treeview\", \"body\":{...}}"]
        if (j.is_array() && j.size() == 1 && j[0].is_string()) {
            j = json::parse(j[0].get<std::string>());
        }

        // JSON에서 panel/body 추출 (없으면 빈 문자열)
        std::string panel = j.value("panel", "");
        std::string body  = j.value("body", "");

        // === window 제어 처리 ===
        if (panel == "window") {

        #if defined(__linux__)
            GtkWindow* gwin = GTK_WINDOW(w.window().value());

            // 1) 간단한 문자열 명령은 바로 처리 (JSON 파싱 X)
            if (body == "close") {
                gtk_window_close(gwin);
                return "{}";
            }
            if (body == "minimize") {
                gtk_window_iconify(gwin);
                return "{}";
            }
            if (body == "maximize") {
                if (!gtk_window_is_maximized(gwin))
                    gtk_window_maximize(gwin);
                else
                    gtk_window_unmaximize(gwin);
                return "{}";
            }

            // 2) 나머지는 드래그용 JSON 으로 간주
            json b;
            try {
                b = json::parse(body);
            } catch (...) {
                // 잘못된 JSON이면 그냥 무시
                return "{}";
            }

            std::string action = b.value("action", "");
            if (action == "begin_move") {
                int button = b.value("button", 1);
                // DOM MouseEvent.button: 0 = left -> GTK: 1 = left
                if (button == 0) button = 1;

                int x = b.value("x", 0);
                int y = b.value("y", 0);

                gtk_window_begin_move_drag(
                    gwin,
                    button,
                    x,
                    y,
                    GDK_CURRENT_TIME   // 타임스탬프는 현재 시간 사용
                );
            }
        #endif

            return "{}";
        }

        // ===== 일반 패널 메시지 처리 =====
        std::string resp = panelManager.dispatch(panel, body);

        // C++ -> JS 메시지 전달
        std::string jsCall =
            "window.handlePanelMessage("
            "'" + panel + "',"
            + resp +
            ");";

        w.eval(jsCall);

        return "{}";
    });



    // index.html 로드
    std::string base = std::filesystem::current_path().string();
    w.navigate("file://" + base + "/../gui/ui/index.html");

#if defined(__linux__)
{
    // webview에서 GtkWindow 포인터 가져오기
    void* raw = w.window().value();
    GtkWidget* gtk_window = GTK_WIDGET(raw);

    // ===== 여기 추가 =====
    // 시스템 타이틀바 제거
    gtk_window_set_decorated(GTK_WINDOW(gtk_window), FALSE);
    gtk_widget_set_app_paintable(gtk_window, TRUE);
    // ======================

    GtkWidget* gtk_webview = gtk_bin_get_child(GTK_BIN(gtk_window));
    WebKitWebView* view = WEBKIT_WEB_VIEW(gtk_webview);
    WebKitSettings* settings = webkit_web_view_get_settings(view);

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
