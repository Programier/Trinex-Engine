cmake_minimum_required(VERSION 3.5)
project(WindowSystemSDL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SDL_SYSTEM_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SDL_SYSTEM_SOURCES_ROOT "${SDL_SYSTEM_PROJECT_ROOT}/src/")
include_directories("include/")
include_directories("${TRINEX_ENGINE_ROOT}/include-internal/")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/libs/")
add_subdirectory(src)
