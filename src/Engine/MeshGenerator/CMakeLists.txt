cmake_minimum_required(VERSION 3.5)
project(MESHGENERATOR LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(flags)

set(MESHGENERATOR_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(MESHGENERATOR_SOURCES_ROOT MESHGENERATOR_PROJECT_ROOT}/src)
include_directories("include/")
include_directories("../include/")
add_subdirectory(src)
