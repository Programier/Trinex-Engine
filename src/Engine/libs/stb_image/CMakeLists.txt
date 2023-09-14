cmake_minimum_required(VERSION 3.5)
project(STB_IMAGE LANGUAGES CXX)

add_library(stb_image STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/implementation.cpp")
set_target_properties(stb_image PROPERTIES COMPILE_OPTIONS "-fPIC")
target_include_directories(stb_image PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>")
