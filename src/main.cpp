#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW Testing", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    std::cout << "OpenGL Version >> " << glGetString(GL_VERSION) << std::endl;

    // 실험모드
    bool useSwap = true;

    float t = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        t += 0.01f;

        // 시간에 따라 배경색 변화
        float r = (sin(t) + 1.0f) / 2.0f;
        float g = (sin(t + 2.0f) + 1.0f) / 2.0f;
        float b = (sin(t + 4.0f) + 1.0f) / 2.0f;

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwPollEvents();

        if (useSwap) {
            glfwSwapBuffers(window);
        }
        // swap 해제시 -> 화면에서 디스코팡팡
    }

    glfwTerminate();
    return 0;
}