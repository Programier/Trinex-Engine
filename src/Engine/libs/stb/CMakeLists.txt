cmake_minimum_required(VERSION 3.5)
project(STB_IMAGE LANGUAGES CXX)

add_library(stb_library STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/implementation.cpp")
set_target_properties(stb_library PROPERTIES COMPILE_OPTIONS "-fPIC")
target_include_directories(stb_library PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>")
