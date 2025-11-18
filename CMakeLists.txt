cmake_minimum_required(VERSION 3.20)
project(OpenGLStudy LANGUAGES C CXX)

# C++ 버전
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include/ 디렉토리 검색
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/glad/include
)

# src/ 안의 모든 cpp 자동 수집
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/glad/src/glad.c
)

# FetchContent
include(FetchContent)

# ======================
# GLFW 자동 다운로드
# ======================
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# pthread & OpenGL
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# 실행 파일 만들기
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${CMAKE_SOURCE_DIR}/glad/src/glad.c   # <- GLAD 로더 C 파일 직접 등록
)

# 링크
target_link_libraries(${PROJECT_NAME}
    glfw
    OpenGL::GL
    Threads::Threads
)

# macOS 특수 처리
if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
