# OpenGLStudy

자동화된 OpenGL 학습용 프로젝트입니다.  
GLFW는 자동으로 다운로드되고, GLAD는 직접 포함하여 CMake로 자동 빌드됩니다.  
`src/` 폴더 안에 `.cpp` 파일만 넣으면 빌드와 실행이 자동으로 이루어집니다.

---

## 프로젝트 구조

```
OpenGLStudy/
├─ CMakeLists.txt
├─ run.sh
├─ error.sh
├─ include/
├─ glad/
│    ├─ include/glad/*.h
│    ├─ include/KHR/*.h
│    └─ src/glad.c
└─ src/
└─ main.cpp
```

---

## 실행 방법

### 1. 빌드 & 실행
```sh
./run.sh src/main.cpp
```

* `build/` 폴더가 자동 생성됨
* CMake -> Make -> 실행까지 자동 처리
* 출력 실행 파일 이름은 `OpenGLStudy`

---

## 오류 추출 (ChatGPT용)

코드 오류를 ChatGPT에 보내고 싶을 때:

```sh
./error.sh src/main.cpp
```

출력 예시:

```cpp
// src/main.cpp
#include <...>
...
```

---

## GLAD 설정 방법

이 프로젝트는 GLAD를 직접 포함해 사용합니다.
GLAD를 새로 생성하려면 아래 절차를 따르십시오:

1. GLAD 공식 사이트: [https://glad.dav1d.de](https://glad.dav1d.de)
2. 설정:

   * **Language**: C/C++
   * **Specification**: OpenGL
   * **API Version**: 4.6
   * **Profile**: Compatibility (전 버전 호환 포함)
   * **Options**: Generate Loader ✓
3. ZIP 다운로드
4. ZIP 안의 `include/` 와 `src/glad.c` 파일을
   프로젝트의 `glad/` 폴더에 그대로 넣기

---

## 빌드 시스템 자동 처리

* `src/` 폴더에 있는 모든 `.cpp` 파일 자동 포함
* `glad/src/glad.c` 자동 포함
* `glad/include` 자동 include path
* GLFW는 CMake FetchContent로 자동 다운로드 및 빌드
* Linux에서는 pthread 자동 링크
* macOS에서는 Cocoa/OpenGL/IOKit 자동 링크

---

## 기본 테스트 코드 예시

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD load failed\n";
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
```

---

## END.
