cmake_minimum_required(VERSION 3.5)
project(IMGUI_GUIZMO LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_sources(${TRINEX_EDITOR_TARGET} PRIVATE 
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/ImGuizmo.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/ImSequencer.cpp)

target_include_directories(${TRINEX_EDITOR_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
