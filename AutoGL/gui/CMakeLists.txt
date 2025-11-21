cmake_minimum_required(VERSION 3.20)

project(AutoGL_GUI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# ImGui (submodule)
# ------------------------------------------------------------------------------
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp

    # backends
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ------------------------------------------------------------------------------
# GUI Layer sources
# ------------------------------------------------------------------------------
set(GUI_SRC
    GuiApp.cpp
    ImGuiLayer.cpp
    panels/FileTreePanel.cpp
    panels/EditorPanel.cpp
    panels/ConsolePanel.cpp
)

# ------------------------------------------------------------------------------
# Final GUI Library
# ------------------------------------------------------------------------------
add_library(AutoGL_GUI STATIC
    ${GUI_SRC}
    ${IMGUI_SRC}
)

target_include_directories(AutoGL_GUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${IMGUI_DIR}                      
        ${IMGUI_DIR}/backends             
)

target_link_libraries(AutoGL_GUI
    PRIVATE
        AutoGL
        glfw
        OpenGL::GL
)
