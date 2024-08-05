cmake_minimum_required(VERSION 3.5)
project(IMGUI_GUIZMO LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_GUIZMO_SOURCES    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImGuizmo.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/ImSequencer.cpp)

add_trinex_editor_sources(${IMGUI_GUIZMO_SOURCES})
add_trinex_editor_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")
