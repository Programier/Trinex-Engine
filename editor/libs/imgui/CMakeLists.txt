cmake_minimum_required(VERSION 3.5)
project(IMGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_sources(${TRINEX_EDITOR_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_stacklayout.cpp)

add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
