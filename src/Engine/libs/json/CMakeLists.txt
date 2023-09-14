cmake_minimum_required(VERSION 3.5)
project(JSON LANGUAGES CXX)

add_library(json INTERFACE)
target_include_directories(json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
