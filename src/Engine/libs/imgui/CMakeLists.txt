cmake_minimum_required(VERSION 3.5)
project(IMGUI LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SOURCES ./src/imgui.cpp
            ./src/imgui_demo.cpp
            ./src/imgui_draw.cpp
            ./src/imgui_tables.cpp
            ./src/imgui_widgets.cpp)
add_definitions("-DIMGUI_EXPORT_ENABLE")
add_library(imgui STATIC ${SOURCES})
set_target_properties(imgui PROPERTIES COMPILE_OPTIONS "-fPIC")

target_include_directories(imgui PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

