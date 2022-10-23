cmake_minimum_required(VERSION 3.5)
project(Editor LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(flags)

set(EDITOR_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(EDITOR_SOURCES_ROOT ${EDITOR_PROJECT_ROOT}/src)
include_directories("include/")
link_directories(libs)

add_subdirectory(src)
