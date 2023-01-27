cmake_minimum_required(VERSION 3.5)
project(OpenGL LANGUAGES CXX)

add_definitions(-DENABLE_OPENGL_EXPORTS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(flags)

set(OPENGL_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(OPENGL_SOURCES_ROOT ${OPENGL_PROJECT_ROOT}/src)

include_directories("${OPENGL_PROJECT_ROOT}/include/")
include_directories("${ENGINE_SRC}/EngineHeaders/")
include_directories("${ENGINE_ROOT}/include/")
add_subdirectory(src)
