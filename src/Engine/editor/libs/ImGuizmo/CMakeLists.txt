cmake_minimum_required(VERSION 3.5)
project(IMGUI_GUIZMO LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_GUIZMO_SOURCES src/GraphEditor.cpp
                        src/ImCurveEdit.cpp
                        src/ImGradient.cpp
                        src/ImGuizmo.cpp
                        src/ImSequencer.cpp)

add_library(imgui_guizmo STATIC ${IMGUI_GUIZMO_SOURCES})
target_link_libraries(imgui_guizmo TrinexEngine)
target_include_directories(imgui_guizmo PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

