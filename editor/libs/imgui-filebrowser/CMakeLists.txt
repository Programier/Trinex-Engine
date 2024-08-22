cmake_minimum_required(VERSION 3.5)
project(IMGUI-FILEBROWSER LANGUAGES CXX)

add_library(imgui-filebrowser INTERFACE)
target_include_directories(imgui-filebrowser INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
