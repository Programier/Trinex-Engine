cmake_minimum_required(VERSION 3.5)
project(Vulkan LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(flags)

set(VULKAN_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(VULKAN_SOURCES_ROOT ${VULKAN_PROJECT_ROOT}/src)
include_directories("include/")
add_subdirectory(src)
