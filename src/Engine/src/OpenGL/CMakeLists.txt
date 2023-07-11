cmake_minimum_required(VERSION 3.5)
project(OPENGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(OPENGL_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(OPENGL_SOURCES_ROOT "${OPENGL_PROJECT_ROOT}/src/")
include_directories("include/")

set(TRINEX_ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../")

include_directories("${TRINEX_ENGINE_ROOT}/include-internal/")
include_directories("${TRINEX_ENGINE_ROOT}/include/")
include_directories("${TRINEX_ENGINE_ROOT}/include/ImGui/")
include_directories("${TRINEX_ENGINE_ROOT}/libs/SDL/include/")

if(WIN32)
    include_directories("${OPENGL_PROJECT_ROOT}/libs/glew-2.2.0/include/")
    add_subdirectory("libs/glew-2.2.0/build/cmake/" EXCLUDE_FROM_ALL)
endif()

add_subdirectory(src)
