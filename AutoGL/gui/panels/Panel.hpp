// panels/Panel.hpp
#pragma once
#include <string>

class Panel {
public:
    virtual ~Panel() {}

    virtual std::string name() const = 0;

    // JS -> C++ 메시지 처리
    virtual std::string onMessage(const std::string& msg) { 
        return "{}"; 
    }

    // 패널이 초기화될 때 JS에게 call을 보내는 용도
    virtual void onLoad() {}

};  