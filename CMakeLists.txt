cmake_minimum_required(VERSION 3.5)
project(TrinexEngineLauncher LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(compilerFlags)

set(GAME_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(GAME_SOURCES_ROOT ${GAME_PROJECT_ROOT}/src)
set(GAME_ENGINE_ROOT ${GAME_SOURCES_ROOT}/Engine/)

set(GAME_COMPILE_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX "${GAME_COMPILE_DIR}/Game/")

if (WIN32)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${GAME_PROJECT_ROOT}/src/Engine/libs/libglew.dll ${CMAKE_INSTALL_PREFIX}/glew32.dll)
endif()

add_subdirectory(src)
target_include_directories(TrinexEngineLauncher PUBLIC "$<BUILD_INTERFACE:${GAME_PROJECT_ROOT}/include>")

install(TARGETS TrinexEngineLauncher DESTINATION "./")

install(DIRECTORY resources DESTINATION "./")
