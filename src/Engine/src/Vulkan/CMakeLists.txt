cmake_minimum_required(VERSION 3.5)
project(Vulkan LANGUAGES CXX)

add_definitions(-DENABLE_VULKAN_EXPORTS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(VULKAN_SOURCES_ROOT ${VULKAN_PROJECT_ROOT}/src)

include_directories("${VULKAN_PROJECT_ROOT}/include/")
include_directories("${TRINEX_ENGINE_ROOT}/include-internal/")
include_directories("${TRINEX_ENGINE_ROOT}/include/")

add_subdirectory(libs/vk-bootstrap)
add_subdirectory(src)
