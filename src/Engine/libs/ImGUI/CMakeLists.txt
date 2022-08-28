cmake_minimum_required(VERSION 3.5)
project(ImGUI LANGUAGES CXX)
set(CMAKE_PROJECT_NAME ImGUI)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(flags)

set(ImGUI_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ImGUI_SOURCES_ROOT ${ImGUI_PROJECT_ROOT}/src)

include_directories("${SDL_DIR}/include")
add_subdirectory(src)


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${ImGUI_PROJECT_ROOT}/include>")
