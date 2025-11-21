# gui cmake
cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(AutoGL_GUI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# FetchContent: webview
# ------------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview.git
    GIT_TAG        master
)

# webview 기본 옵션 설정 (정적 링크)
set(WEBVIEW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WEBVIEW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WEBVIEW_BUILD_SHARED OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(webview)

# ------------------------------------------------------------------------------
# FetchContent: nlohmann-json
# ------------------------------------------------------------------------------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

# ------------------------------------------------------------------------------
# GUI Sources
# ------------------------------------------------------------------------------
set(GUI_SRC
    WebViewLayer.cpp
    GuiApp.cpp
)

# ------------------------------------------------------------------------------
# GUI Library
# ------------------------------------------------------------------------------
add_library(AutoGL_GUI STATIC
    ${GUI_SRC}
)

target_include_directories(AutoGL_GUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${webview_BINARY_DIR}/include           # webview.h 인식 위해 반드시 필요
)

# ------------------------------------------------------------------------------
# Platform-specific Dependencies
# ------------------------------------------------------------------------------
# Linux -> WebKitGTK-4.0 필요
# Windows -> WebView2
# macOS -> Cocoa + WebKit (추가 가능)

if(WIN32)
    message(STATUS "Configuring AutoGL_GUI for Windows using WebView2")

    target_compile_definitions(AutoGL_GUI PRIVATE WEBVIEW_EDGE)

    # webview::core_static 또는 webview::core 사용
    target_link_libraries(AutoGL_GUI
        PRIVATE
            webview::core_static
    )

elseif(APPLE)
    message(STATUS "Configuring AutoGL_GUI for macOS using Cocoa/WebKit")

    target_compile_definitions(AutoGL_GUI PRIVATE WEBVIEW_COCOA)

    target_link_libraries(AutoGL_GUI
        PRIVATE
            webview::core_static
            "-framework Cocoa"
            "-framework WebKit"
    )

else()  # Linux (GTK)
    message(STATUS "Configuring AutoGL_GUI for Linux using WebKitGTK")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)

    target_compile_definitions(AutoGL_GUI PRIVATE WEBVIEW_GTK)

    target_include_directories(AutoGL_GUI PRIVATE ${WEBKIT2GTK_INCLUDE_DIRS})
    target_link_directories(AutoGL_GUI PRIVATE ${WEBKIT2GTK_LIBRARY_DIRS})

    target_link_libraries(AutoGL_GUI
        PRIVATE
            webview::core_static          # webview 정적 링크
            ${WEBKIT2GTK_LIBRARIES}       # WebKitGTK
            dl                            # webview가 Linux에서 필요함
    )
endif()

# ------------------------------------------------------------------------------
# Link AutoGL (core engine)
# ------------------------------------------------------------------------------
target_link_libraries(AutoGL_GUI
    PRIVATE
        AutoGL
        nlohmann_json::nlohmann_json
)
