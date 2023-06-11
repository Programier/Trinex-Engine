cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(VulkanBootstrap)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BOOTSTRAP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options("-fPIC")
add_library(vk-bootstrap STATIC ${BOOTSTRAP_ROOT}/src/VkBootstrap.cpp)
target_include_directories(vk-bootstrap PUBLIC  "$<BUILD_INTERFACE:${BOOTSTRAP_ROOT}/include>")
