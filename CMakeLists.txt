cmake_minimum_required(VERSION 3.20)
project(OpenGLStudy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include dirs
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/glad/include
)

# src cpp 자동 스캔
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# glad
set(GLAD_SRC ${CMAKE_SOURCE_DIR}/glad/src/glad.c)

# Fetch GLFW
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# -------------------------------
# OpenGLStudy 실행 파일
# -------------------------------
add_executable(OpenGLStudy
    ${SOURCES}
    ${GLAD_SRC}
)

target_link_libraries(OpenGLStudy
    glfw
    OpenGL::GL
    Threads::Threads
)

# -------------------------------
# AutoGL 프로젝트 추가
# -------------------------------
add_subdirectory(AutoGL)

# macOS frameworks
if(APPLE)
    target_link_libraries(OpenGLStudy
        "-framework Cocoa" "-framework OpenGL"
        "-framework IOKit" "-framework CoreVideo")
endif()
