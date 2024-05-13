cmake_minimum_required(VERSION 3.5)
project(IMGUI LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(TRINEX_EXTERNAL_SOURCES ${TRINEX_EXTERNAL_SOURCES}
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_demo.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_draw.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_tables.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_widgets.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_stacklayout.cpp PARENT_SCOPE)

add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

