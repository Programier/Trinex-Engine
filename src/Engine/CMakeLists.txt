cmake_minimum_required(VERSION 3.5)
project(TrinexEngine LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(flags)


add_definitions(-DENABLE_ENGINE_EXPORTS)
add_definitions(-DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DTRINEX_ENGINE_DEBUG)
endif()

add_definitions(-DENGINE_API_IMPLEMENTATION)

#Compile parameters
set(TRINEX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_SRC "${TRINEX_ENGINE_ROOT}/src")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Compile freetype library
set(BUILD_SHARED_LIBS off)
add_subdirectory("${TRINEX_ENGINE_ROOT}/libs/freetype/" EXCLUDE_FROM_ALL)
set_target_properties(freetype PROPERTIES COMPILE_OPTIONS "-fPIC")


# Compile SDL2 library
set(BUILD_SHARED_LIBS on)
set(SDL_SHARED on)
set(SDL_DIR "${TRINEX_ENGINE_ROOT}/libs/SDL")
add_subdirectory(${SDL_DIR} EXCLUDE_FROM_ALL)
include_directories("${SDL_DIR}/include")

# Compile LZ4 library
set(BUILD_SHARED_LIBS off)
add_subdirectory("${TRINEX_ENGINE_ROOT}/libs/lz4/build/cmake/")

# Include Trinex Engine headers directories
include_directories("${TRINEX_ENGINE_ROOT}/include/")
include_directories("${TRINEX_ENGINE_ROOT}/include-internal/")

# Set source directories
set(TRINEX_WINDOW_DIR "${TRINEX_ENGINE_SRC}/Window/")
set(TRINEX_EVENT_DIR "${TRINEX_ENGINE_SRC}/Event/")
set(TRINEX_IMAGE_DIR "${TRINEX_ENGINE_SRC}/Image/")
set(TRINEX_GRAPHICS_DIR "${TRINEX_ENGINE_SRC}/Graphics/")
set(TRINEX_PHYSICS_DIR "${TRINEX_ENGINE_SRC}/Physics/")
set(TRINEX_CORE_DIR "${TRINEX_ENGINE_SRC}/Core/")
set(TRINEX_NET_DIR "${TRINEX_ENGINE_SRC}/net/")
set(TRINEX_LIB_LOADER_DIR "${TRINEX_ENGINE_SRC}/LibLoader/")
set(TRINEX_IMGUI_DIR "${TRINEX_ENGINE_SRC}/ImGui/")
set(TRINEX_SENSORS_DIR "${TRINEX_ENGINE_SRC}/Sensors/")
set(TRINEX_COMMANDLETS_DIR "${TRINEX_ENGINE_SRC}/CommandLets/")

# Find sources

file(GLOB TRINEX_WINDOW_SRC "${TRINEX_WINDOW_DIR}*.cpp")
file(GLOB TRINEX_IMAGE_SRC "${TRINEX_IMAGE_DIR}*.cpp")
file(GLOB TRINEX_GRAPHICS_SRC "${TRINEX_GRAPHICS_DIR}*.cpp")
file(GLOB TRINEX_PHYSICS_SRC "${TRINEX_PHYSICS_DIR}*.cpp")
file(GLOB TRINEX_CORE_SRC "${TRINEX_CORE_DIR}*.cpp")
file(GLOB TRINEX_NET_SRC "${TRINEX_NET_DIR}*.cpp")
file(GLOB TRINEX_LIB_LOADER_SRC "${TRINEX_LIB_LOADER_DIR}*.cpp")
file(GLOB TRINEX_IMGUI_SRC "${TRINEX_IMGUI_DIR}*.cpp")
file(GLOB TRINEX_EVENT_SRC "${TRINEX_EVENT_DIR}*.cpp")
file(GLOB TRINEX_SENSORS_SRC "${TRINEX_SENSORS_DIR}*.cpp")
file(GLOB TRINEX_COMMANDLETS_SRC "${TRINEX_COMMANDLETS_DIR}*.cpp")

# Write all source files to one variable
set(TRINEX_ENGINE_SOURCE_FILES ${TRINEX_WINDOW_SRC}
                        ${TRINEX_IMAGE_SRC}
                        ${TRINEX_GRAPHICS_SRC}
                        ${TRINEX_PHYSICS_SRC}
                        ${TRINEX_CORE_SRC}
                        ${TRINEX_LIB_LOADER_SRC}
                        ${TRINEX_IMGUI_SRC}
                        ${TRINEX_EVENT_SRC}
                        ${TRINEX_SENSORS_SRC}
                        ${TRINEX_COMMANDLETS_SRC})


# Include private headers for imgui
include_directories(${TRINEX_ENGINE_ROOT}/include-internal/ImGui-Private/)

# Include additional lib directory and include dir if build system is Windows OS
if (WIN32)
    link_directories("${TRINEX_ENGINE_ROOT}/libs/")
endif()

# Compile OpenGL library
add_subdirectory("${TRINEX_ENGINE_SRC}/OpenGL")

# Compile Vulkan library (Windows OS currently not supported)

add_subdirectory("${TRINEX_ENGINE_SRC}/Vulkan")


# Compile Trinex Engine library
add_library(TrinexEngine SHARED ${TRINEX_ENGINE_SOURCE_FILES})

# Linking libraries and including directories
target_include_directories(TrinexEngine PUBLIC "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/include>"
                                         "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/include/ImGui>"
                                         "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/libs/LuaJIT/include/>")
target_link_libraries(TrinexEngine PRIVATE SDL2 freetype lz4_static ${PlatformLibs})
