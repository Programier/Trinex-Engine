cmake_minimum_required(VERSION 3.5)
project(TrinexEngine LANGUAGES CXX)


option(TRINEX_WITH_VULKAN "Build Vulkan API shared library" ON)
option(TRINEX_WITH_OPENGL "Build OpenGL API shared library" ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#Compile parameters
set(TRINEX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_SRC "${TRINEX_ENGINE_ROOT}/src")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Compile freetype library
set(BUILD_SHARED_LIBS off)
set(FT_DISABLE_HARFBUZZ on)
add_subdirectory("${TRINEX_ENGINE_ROOT}/libs/freetype/" EXCLUDE_FROM_ALL)
set_target_properties(freetype PROPERTIES COMPILE_OPTIONS "-fPIC")

# Compile LuaJIT

add_subdirectory("${TRINEX_ENGINE_ROOT}/libs/LuaJIT/")


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
include_directories("${TRINEX_ENGINE_ROOT}/libs/LuaJIT/include/")

# Set source directories
set(TRINEX_WINDOW_DIR "${TRINEX_ENGINE_SRC}/Window/")
set(TRINEX_EVENT_DIR "${TRINEX_ENGINE_SRC}/Event/")
set(TRINEX_IMAGE_DIR "${TRINEX_ENGINE_SRC}/Image/")
set(TRINEX_GRAPHICS_DIR "${TRINEX_ENGINE_SRC}/Graphics/")
set(TRINEX_PHYSICS_DIR "${TRINEX_ENGINE_SRC}/Physics/")
set(TRINEX_CORE_DIR "${TRINEX_ENGINE_SRC}/Core/")
set(TRINEX_GLM_BINDINGS_DIR "${TRINEX_ENGINE_SRC}/Core/glm_bindings/")
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
file(GLOB TRINEX_GLM_BINDINGS_SRC "${TRINEX_GLM_BINDINGS_DIR}*.cpp")
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
                        ${TRINEX_COMMANDLETS_SRC}
                        ${TRINEX_GLM_BINDINGS_SRC})


include_directories("libs/tinygltf/include/")
set(TRINEX_TINYGLTF_DIR "${TRINEX_ENGINE_SRC}/tinygltf/")
file(GLOB TRINEX_TINYGLTF_SRC "${TRINEX_TINYGLTF_DIR}*.cpp")
set(TRINEX_ENGINE_SOURCE_FILES ${TRINEX_ENGINE_SOURCE_FILES} ${TRINEX_TINYGLTF_SRC})



# Include private headers for imgui
include_directories(${TRINEX_ENGINE_ROOT}/include-internal/ImGui-Private/)

# Compile OpenGL library
if(TRINEX_WITH_OPENGL)
    add_subdirectory("${TRINEX_ENGINE_SRC}/OpenGL")
endif()

# Compile Vulkan library (Windows OS currently not supported)
if(TRINEX_WITH_VULKAN)
    add_subdirectory("${TRINEX_ENGINE_SRC}/Vulkan")
endif()



# Compile Trinex Engine library
add_library(TrinexEngine SHARED ${TRINEX_ENGINE_SOURCE_FILES})


# Trinex Engine compile options
target_compile_options(TrinexEngine PRIVATE "-Wall")
target_compile_options(TrinexEngine PRIVATE "-Wno-unknown-pragmas")
target_compile_options(TrinexEngine PRIVATE "-Wmismatched-tags")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(TrinexEngine PRIVATE "-O3")
    target_compile_options(TrinexEngine PRIVATE "-Os")
    target_compile_options(TrinexEngine PRIVATE "-finline-functions")
    target_compile_options(TrinexEngine PRIVATE "-funroll-loops")
    target_compile_options(TrinexEngine PRIVATE "-fomit-frame-pointer")
else()
    target_compile_options(TrinexEngine PRIVATE "-g")
endif()

# Trinex Engine compile definitions
target_compile_definitions(TrinexEngine PRIVATE -DENABLE_ENGINE_EXPORTS)
target_compile_definitions(TrinexEngine PRIVATE -DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_DEBUG_BUILD=1)
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_RELEASE_BUILD=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_DEBUG_BUILD=0)
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_RELEASE_BUILD=1)
endif()


# Include additional lib directory and include dir if build system is Windows OS
if (WIN32)
    target_link_directories(TrinexEngine "${TRINEX_ENGINE_ROOT}/libs/")
endif()


# Linking libraries and including directories
target_include_directories(TrinexEngine PUBLIC "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/include>"
                                         "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/include/ImGui>"
                                         "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/libs/LuaJIT/include/>")

if(ANDROID)
    set(LOG_LIBS android log)
endif()

target_link_libraries(TrinexEngine PRIVATE SDL2 freetype lz4_static libluajit ${LOG_LIBS})


install(TARGETS TrinexEngine SDL2 DESTINATION "./TrinexEngine/libs/")
install(DIRECTORY configs DESTINATION "./TrinexEngine/")

