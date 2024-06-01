cmake_minimum_required(VERSION 3.5)
project(Vulkan LANGUAGES CXX)

add_definitions(-DENABLE_VULKAN_EXPORTS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(VULKAN_SOURCES_ROOT ${VULKAN_PROJECT_ROOT}/src)

include_directories("${VULKAN_PROJECT_ROOT}/include/")
include_directories("${TRINEX_ENGINE_ROOT}/include-internal/")
include_directories("${VULKAN_PROJECT_ROOT}/vulkan_headers/")

add_subdirectory(libs/vk-bootstrap)

if(ANDROID)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^aarch64")
        message("LINK DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/android/arm64-v8a/")
        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/android/arm64-v8a/)
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/android/x86_64/)
    endif()
endif()

add_subdirectory(src)
