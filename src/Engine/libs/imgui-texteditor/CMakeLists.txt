cmake_minimum_required(VERSION 3.5)
project(IMGUI LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES ./src/text_editor.cpp)
add_definitions("-DIMGUI_EXPORT_ENABLE")
include_directories("../imgui/include/")

add_library(imgui_texteditor STATIC ${SOURCES})
target_link_libraries(imgui_texteditor imgui)
set_target_properties(imgui_texteditor PROPERTIES COMPILE_OPTIONS "-fPIC")
target_include_directories(imgui_texteditor PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")


