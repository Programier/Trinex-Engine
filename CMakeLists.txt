cmake_minimum_required(VERSION 3.5)
project(Engine3D_Game LANGUAGES CXX)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(compilerFlags)

set(GAME_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(GAME_SOURCES_ROOT ${GAME_PROJECT_ROOT}/src)
set(GAME_CMAKE_INSTALL_PREFIX ${GAME_PROJECT_ROOT}/Exec/)
set(GAME_ENGINE_ROOT ${GAME_SOURCES_ROOT}/Engine/)

set(GAME_COMPILE_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX "${GAME_COMPILE_DIR}/Game/")

set(BUILD_OPENGL on)

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${GAME_PROJECT_ROOT}/resources ${CMAKE_INSTALL_PREFIX}/resources)

if (WIN32)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${GAME_PROJECT_ROOT}/src/Engine/libs/libglew.dll ${CMAKE_INSTALL_PREFIX}/glew32.dll)
endif()

add_subdirectory(src)
target_include_directories(TrinexEngineLauncher PUBLIC "$<BUILD_INTERFACE:${GAME_PROJECT_ROOT}/include>")

#install(DIRECTORY ${GAME_PROJECT_ROOT}/resources DESTINATION "./")
#install(DIRECTORY ${GAME_PROJECT_ROOT}/src/Shaders DESTINATION "./")

install(TARGETS TrinexEngineLauncher DESTINATION "./")

if (ANDROID OR (CMAKE_BUILD_TYPE STREQUAL "Release"))
    install(DIRECTORY resources DESTINATION "./")
    install(DIRECTORY scripts DESTINATION "./")
else()
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${GAME_PROJECT_ROOT}/resources ${CMAKE_INSTALL_PREFIX}/resources)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${GAME_PROJECT_ROOT}/scripts ${CMAKE_INSTALL_PREFIX}/scripts)
endif()
