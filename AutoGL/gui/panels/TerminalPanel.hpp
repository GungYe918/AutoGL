#pragma once
#include "Panel.hpp"
#include <nlohmann/json.hpp>

#include <pty.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <thread>
#include <atomic>
#include <functional>
#include <cerrno>

class TerminalPanel : public Panel {
public:
    TerminalPanel() {
        startShell();
    }

    std::string name() const override {
        return "terminal";
    }

    std::string onMessage(const std::string& msg) override {
        // JS 쪽에서 첫 초기화 완료 신호
        if (msg == "__ready__") {
            bool expected = false;
            if (readerStarted.compare_exchange_strong(expected, true)) {
                reader = std::thread([this] { readLoop(); });
                reader.detach();
            }
            return "{}";
        }

        // 일반 명령어 -> zsh 로 전달
        if (master_fd >= 0) {
            std::string line = msg;
            line.push_back('\n');
            ssize_t ignored = write(master_fd, line.c_str(), line.size());
            (void)ignored;
        }

        return "{}";
    }

    // WebViewLayer에서 세팅해주는 JS eval 함수
    std::function<void(const std::string&)> webviewEval;

private:
    int master_fd = -1;
    pid_t pid = -1;
    std::thread reader;
    std::atomic<bool> readerStarted{false};

    void startShell() {
        pid = forkpty(&master_fd, nullptr, nullptr, nullptr);
        if (pid == 0) {
            // child: interactive login zsh
            execl("/usr/bin/zsh", "zsh", "-i", "-l", (char*)nullptr);
            _exit(1);
        }

        fcntl(master_fd, F_SETFL, O_NONBLOCK);
    }

    void readLoop() {
        char buf[4096];

        for (;;) {
            if (master_fd < 0) break;

            ssize_t n = read(master_fd, buf, sizeof(buf));
            if (n > 0) {
                std::string output(buf, n);

                nlohmann::json resp;
                resp["output"] = output;

                std::string js =
                    "window.handlePanelMessage('terminal'," +
                    resp.dump() +
                    ");";

                if (webviewEval) {
                    webviewEval(js);
                }
            } else if (n == 0) {
                // EOF: zsh 종료
                break;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(10000); // 데이터 없으면 잠깐 쉼
                    continue;
                } else {
                    // 진짜 에러
                    break;
                }
            }
        }
    }
};
