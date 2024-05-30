cmake_minimum_required(VERSION 3.5)
project(IMGUI_NODE_EDITOR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_NODE_EDITOR_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(IMGUI_NODE_EDITOR_SOURCES_ROOT ${IMGUI_NODE_EDITOR_PROJECT_ROOT}/src)

set(TRINEX_ENGINE_SOURCE_FILES ${TRINEX_ENGINE_SOURCE_FILES}
                               ${IMGUI_NODE_EDITOR_SOURCES_ROOT}/crude_json.cpp
                               ${IMGUI_NODE_EDITOR_SOURCES_ROOT}/imgui_canvas.cpp
                               ${IMGUI_NODE_EDITOR_SOURCES_ROOT}/imgui_node_editor.cpp
                               ${IMGUI_NODE_EDITOR_SOURCES_ROOT}/imgui_node_editor_api.cpp PARENT_SCOPE)

add_library(imgui_node_editor INTERFACE)
target_include_directories(imgui_node_editor INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
