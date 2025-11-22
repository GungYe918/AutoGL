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
        ${webview_BINARY_DIR}/include
)

# ------------------------------------------------------------------------------
# Platform-specific Dependencies
# ------------------------------------------------------------------------------
if(WIN32)
    message(STATUS "[GUI] Windows: WebView2 backend")
    target_compile_definitions(AutoGL_GUI PRIVATE WEBVIEW_EDGE)
    target_link_libraries(AutoGL_GUI PRIVATE webview::core_static)

elseif(APPLE)
    message(STATUS "[GUI] macOS: Cocoa/WebKit backend")
    target_compile_definitions(AutoGL_GUI PRIVATE WEBVIEW_COCOA)
    target_link_libraries(AutoGL_GUI
        PRIVATE
            webview::core_static
            "-framework Cocoa"
            "-framework WebKit"
    )

else()
    message(STATUS "[GUI] Linux: WebKitGTK backend")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)

    target_compile_definitions(AutoGL_GUI PRIVATE WEBVIEW_GTK)
    target_include_directories(AutoGL_GUI PRIVATE ${WEBKIT2GTK_INCLUDE_DIRS})
    target_link_directories(AutoGL_GUI PRIVATE ${WEBKIT2GTK_LIBRARY_DIRS})

    target_link_libraries(AutoGL_GUI
        PRIVATE
            webview::core_static
            ${WEBKIT2GTK_LIBRARIES}
            dl
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

# ------------------------------------------------------------------------------
# Auto-generate import list for main.js
# ------------------------------------------------------------------------------
message(STATUS "[JS] Generating auto-import list for main.js")

set(JS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui/js")
set(MAIN_JS "${JS_DIR}/main.js")
set(TMP_MAIN_JS "${JS_DIR}/main.tmp.js")

# JS 모듈 목록을 스캔한다 (main.js, main.tmp.js 는 제외)
file(GLOB_RECURSE JS_MODULES
    RELATIVE "${JS_DIR}"
    "${JS_DIR}/*.js"
)
list(REMOVE_ITEM JS_MODULES "main.js" "main.tmp.js")

# 중복되는 파일 제거
list(REMOVE_DUPLICATES JS_MODULES)

message(STATUS "[JS] Found JS modules:")
foreach(MOD ${JS_MODULES})
    message(STATUS "   JS module: ${MOD}")
endforeach()

# 자동 import 코드 문자열 생성
set(AUTO_IMPORT_CODE "")
foreach(MOD ${JS_MODULES})
    # 경로 정규화
    string(REPLACE "\\" "/" MOD_PATH "${MOD}")

    # .js 제거
    string(REPLACE ".js" "" MOD_ID_TMP "${MOD_PATH}")

    # 폴더 구분자 '/' 를 '_' 로 바꿔서 JS 식별자에 안전한 이름 생성
    string(REPLACE "/" "_" MOD_ID "${MOD_ID_TMP}")

    # import 구문 생성 (중복 없이 안정적으로 append)
    string(APPEND AUTO_IMPORT_CODE
        "import * as mod_${MOD_ID} from \"./${MOD_PATH}\";\n"
    )
    string(APPEND AUTO_IMPORT_CODE
        "window.AutoGL_LoadedModules[\"${MOD_PATH}\"] = mod_${MOD_ID};\n\n"
    )

endforeach()

# main.js 템플릿 읽기
file(READ "${MAIN_JS}" MAIN_CONTENTS)

# 마커 문자열 정의
set(START_MARK "// __AUTOGL_IMPORTS_START__")
set(END_MARK   "// __AUTOGL_IMPORTS_END__")

# 마커 위치 찾기
string(FIND "${MAIN_CONTENTS}" "${START_MARK}" START_POS)
string(FIND "${MAIN_CONTENTS}" "${END_MARK}"   END_POS)

if(START_POS EQUAL -1 OR END_POS EQUAL -1)
    message(FATAL_ERROR "[JS] main.js import markers not found.")
endif()

# 길이 계산
string(LENGTH "${START_MARK}" START_LEN)
string(LENGTH "${END_MARK}"   END_LEN)
string(LENGTH "${MAIN_CONTENTS}" MAIN_LEN)

math(EXPR END_POS_PLUS_LEN "${END_POS} + ${END_LEN}")
math(EXPR SUFFIX_LEN "${MAIN_LEN} - ${END_POS_PLUS_LEN}")

# prefix: START_MARK 이전까지
string(SUBSTRING "${MAIN_CONTENTS}" 0 "${START_POS}" PREFIX)
# suffix: END_MARK 이후 나머지 전체
string(SUBSTRING "${MAIN_CONTENTS}" "${END_POS_PLUS_LEN}" "${SUFFIX_LEN}" SUFFIX)

# 새 main 내용 구성
set(NEW_MAIN_CONTENTS
"${PREFIX}${START_MARK}\n${AUTO_IMPORT_CODE}${END_MARK}${SUFFIX}"
)

# configure 시점에 main.tmp.js를 생성한다
file(WRITE "${TMP_MAIN_JS}" "${NEW_MAIN_CONTENTS}")

set(JS_MAIN_FILE "${TMP_MAIN_JS}" CACHE INTERNAL "")

message(STATUS "[JS] main.tmp.js generated: ${TMP_MAIN_JS}")

# ------------------------------------------------------------------------------
# JavaScript Bundling (esbuild - CLI 직접 호출)
# ------------------------------------------------------------------------------
set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ui")
set(JS_OUTPUT "${UI_DIR}/bundle.js")

message(STATUS "[JS] Configuring JS bundling (configure-time)")

find_program(NPM_EXECUTABLE npm)
find_program(ESBUILD_EXECUTABLE esbuild)

if(NPM_EXECUTABLE)
    message(STATUS "[JS] npm found: ${NPM_EXECUTABLE}")
else()
    message(WARNING "[JS] npm not found. Install Node.js and npm first.")
endif()

if(ESBUILD_EXECUTABLE)
    message(STATUS "[JS] esbuild found: ${ESBUILD_EXECUTABLE}")
else()
    message(WARNING "[JS] esbuild not found. Install with: npm install -g esbuild")
endif()

if(ESBUILD_EXECUTABLE)
    # ui 디렉토리를 기준으로 한 main.tmp.js 상대 경로를 만든다
    file(RELATIVE_PATH JS_MAIN_REL "${UI_DIR}" "${JS_MAIN_FILE}")
    message(STATUS "[JS] esbuild entry: ${JS_MAIN_REL}")
    message(STATUS "[JS] esbuild output: ${JS_OUTPUT}")

    execute_process(
        COMMAND "${ESBUILD_EXECUTABLE}"
            "${JS_MAIN_REL}"
            --bundle
            --format=iife
            --global-name=AutoGLBundle
            "--outfile=${JS_OUTPUT}"
            --sourcemap
        WORKING_DIRECTORY "${UI_DIR}"
        RESULT_VARIABLE ESBUILD_RESULT
        OUTPUT_VARIABLE ESBUILD_STDOUT
        ERROR_VARIABLE  ESBUILD_STDERR
    )

    if(ESBUILD_RESULT EQUAL 0)
        message(STATUS "[JS] esbuild bundling succeeded")
    else()
        message(WARNING "[JS] esbuild bundling failed with code ${ESBUILD_RESULT}")
        message(WARNING "[JS] esbuild stdout:")
        message(WARNING "${ESBUILD_STDOUT}")
        message(WARNING "[JS] esbuild stderr:")
        message(WARNING "${ESBUILD_STDERR}")
    endif()
else()
    message(WARNING "[JS] esbuild missing. bundle.js will not be generated.")
endif()
