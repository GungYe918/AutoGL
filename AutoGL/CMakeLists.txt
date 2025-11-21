# AutoGL cmake (root cmake)
cmake_minimum_required(VERSION 3.20)
project(AutoGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------
# ccache 자동 감지
# ------------------------------------------------------------------
find_program(CCACHE_PROGRAM ccache)

if (CCACHE_PROGRAM)
    message(STATUS "ccache found: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
else()
    message(STATUS "ccache not found → normal build")
endif()

# ----------------------------------------
# OpenGL + Threads
# ----------------------------------------
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# ----------------------------------------
# GLFW 처리
# ----------------------------------------
# 만약 부모 프로젝트에서 glfw 타겟이 존재하면 그대로 사용
# 없으면 단독 모드로 FetchContent 실행
if(NOT TARGET glfw)
    message(STATUS "AutoGL: glfw target not found -> FetchContent downloading")

    include(FetchContent)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG latest
    )
    FetchContent_MakeAvailable(glfw)
else()
    message(STATUS "AutoGL: using glfw from parent project")
endif()

# ----------------------------------------
# Public headers
# ----------------------------------------
set(AUTOGL_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AutoGL/AutoGL.hpp
)

# ----------------------------------------
# GUI Modules
# ----------------------------------------
# webview FetchContent는 gui/CMakeLists에서만 처리하도록 유지
add_subdirectory(gui)

# ----------------------------------------
# GLAD (AutoGL 내부 전용)
# ----------------------------------------
set(AUTOGL_GLAD_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
)

set(AUTOGL_GLAD_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c
)

# ----------------------------------------
# Internal sources
# ----------------------------------------
set(AUTOGL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/autogl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vk_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glsl_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_regex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_shader_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_shader_vertex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_shader_fragment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl_shader_compute.cpp
    ${AUTOGL_GLAD_SRC}
)

# ----------------------------------------
# Library target
# ----------------------------------------
add_library(AutoGL STATIC
    ${AUTOGL_PUBLIC_HEADERS}
    ${AUTOGL_SOURCES}
)

# ----------------------------------------
# Include directories
# ----------------------------------------
target_include_directories(AutoGL
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${AUTOGL_GLAD_INCLUDE}
)

# ----------------------------------------
# Link dependencies
# ----------------------------------------
target_link_libraries(AutoGL
    glfw
    OpenGL::GL
    Threads::Threads
)

# macOS
if(APPLE)
    target_link_libraries(AutoGL
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# ----------------------------------------
# AutoGL 실행 파일 (autogl)
# ----------------------------------------
add_executable(autogl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(autogl_gui
    gui/main.cpp
)

target_include_directories(autogl
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 엔진 테스트용 실행 파일: AutoGL만 링크
target_link_libraries(autogl
    PRIVATE
        AutoGL
        glfw
        OpenGL::GL
        Threads::Threads
)

# GUI 실행 파일: GUI + 엔진 + 의존성 전부 링크
target_link_libraries(autogl_gui
    PRIVATE
        AutoGL_GUI      # <- GuiApp, WebViewLayer, Panel들
        AutoGL          # <- 엔진
        glfw
        OpenGL::GL
        Threads::Threads
)

if(APPLE)
    target_link_libraries(autogl
        PRIVATE
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
    )
    target_link_libraries(autogl_gui
        PRIVATE
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
    )
endif()
