#include <AutoGL/AutoGL.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3 || std::string(argv[1]) != "--shader") {
        std::cout << "Usage: autogl --shader <shader.glsl>\n";
        return 1;
    }

    std::string path = argv[2];

    AutoGL::Engine engine;
    if (!engine.initGL())
        return 1;

    engine.mainLoop(path);
    return 0;
}
