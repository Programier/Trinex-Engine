cmake_minimum_required(VERSION 3.5)
project(TINYGLTF LANGUAGES CXX)

add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
