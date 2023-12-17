cmake_minimum_required(VERSION 3.5)
project(TrinexEngine LANGUAGES CXX)


option(TRINEX_WITH_VULKAN "Build Vulkan API shared library" ON)
option(TRINEX_WITH_OPENGL "Build OpenGL API shared library" ON)
option(TRINEX_WITH_SDL "Build SDL2 WindowSystem shared library" ON)
option(TRINEX_WITH_SHADER_COMPILERS "Build engine with shader compilers" OFF)
option(TRINEX_WITH_EDITOR "Build engine with editor" ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(TRINEX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_SRC "${TRINEX_ENGINE_ROOT}/src")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(TRINEX_LIBRARIES_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/libs/")
set(TRINEX_INCLUDES_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/")
set(TRINEX_CONFIGS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/")
set(TRINEX_SHADERS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/")

# Global private include directories
include_directories("${TRINEX_ENGINE_ROOT}/include-internal/"
                    "${TRINEX_ENGINE_ROOT}/libs/angelscript/include/")

# Compile RHI
add_subdirectory("${TRINEX_ENGINE_ROOT}/RHI/")

# Compile Window system interfaces
add_subdirectory("${TRINEX_ENGINE_ROOT}/WSI/")

# Compile Third-Party libraries
add_subdirectory("${TRINEX_ENGINE_ROOT}/libs/")

# List of libraries, which TrinexEngine use
set(PRIVATE_THIRD_PARTY_LIBRARIES   freetype
                                    lz4_static
                                    angelscript
                                    stb_image
                                    tinygltf
                                    asmjit)

set(PUBLIC_THIRD_PARTY_LIBRARIES imgui json imgui_node_editor)

if(ANDROID)
    set(PLATFORM_DEPENDED_LIBRARIES android log)
endif()

# Set source directories
set(TRINEX_SRC_WINDOW_DIR "${TRINEX_ENGINE_SRC}/Window/")
set(TRINEX_SRC_EVENT_DIR "${TRINEX_ENGINE_SRC}/Event/")
set(TRINEX_SRC_IMAGE_DIR "${TRINEX_ENGINE_SRC}/Image/")
set(TRINEX_SRC_GRAPHICS_DIR "${TRINEX_ENGINE_SRC}/Graphics/")
set(TRINEX_SRC_PHYSICS_DIR "${TRINEX_ENGINE_SRC}/Physics/")
set(TRINEX_SRC_CORE_DIR "${TRINEX_ENGINE_SRC}/Core/")
set(TRINEX_SRC_NET_DIR "${TRINEX_ENGINE_SRC}/net/")
set(TRINEX_SRC_LIB_LOADER_DIR "${TRINEX_ENGINE_SRC}/LibLoader/")
set(TRINEX_SRC_SENSORS_DIR "${TRINEX_ENGINE_SRC}/Sensors/")
set(TRINEX_SRC_COMMANDLETS_DIR "${TRINEX_ENGINE_SRC}/CommandLets/")
set(TRINEX_SRC_SCRIPT_ENGINE_DIR "${TRINEX_ENGINE_SRC}/ScriptEngine/")
set(TRINEX_SRC_SYSTEMS_DIR "${TRINEX_ENGINE_SRC}/Systems/")
set(TRINEX_SRC_ENGINE_DIR "${TRINEX_ENGINE_SRC}/Engine/")


# Find sources
file(GLOB_RECURSE TRINEX_SRC_WINDOW_SRC "${TRINEX_SRC_WINDOW_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_IMAGE_SRC "${TRINEX_SRC_IMAGE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_GRAPHICS_SRC "${TRINEX_SRC_GRAPHICS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_PHYSICS_SRC "${TRINEX_SRC_PHYSICS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_CORE_SRC "${TRINEX_SRC_CORE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_NET_SRC "${TRINEX_SRC_NET_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_EVENT_SRC "${TRINEX_SRC_EVENT_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_SENSORS_SRC "${TRINEX_SRC_SENSORS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_COMMANDLETS_SRC "${TRINEX_SRC_COMMANDLETS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_SCRIPT_ENGINE_SRC "${TRINEX_SRC_SCRIPT_ENGINE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_SYSTEMS_SRC "${TRINEX_SRC_SYSTEMS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_ENGINE_SRC "${TRINEX_SRC_ENGINE_DIR}*.cpp")

# Write all source files to one variable
set(TRINEX_ENGINE_SOURCE_FILES ${TRINEX_SRC_WINDOW_SRC}
                        ${TRINEX_SRC_IMAGE_SRC}
                        ${TRINEX_SRC_GRAPHICS_SRC}
                        ${TRINEX_SRC_PHYSICS_SRC}
                        ${TRINEX_SRC_CORE_SRC}
                        ${TRINEX_SRC_EVENT_SRC}
                        ${TRINEX_SRC_SENSORS_SRC}
                        ${TRINEX_SRC_COMMANDLETS_SRC}
                        ${TRINEX_SRC_SCRIPT_ENGINE_SRC}
                        ${TRINEX_SRC_SYSTEMS_SRC}
                        ${TRINEX_SRC_ENGINE_SRC})




set(TRINEX_TINYGLTF_DIR "${TRINEX_ENGINE_SRC}/tinygltf/")
file(GLOB TRINEX_TINYGLTF_SRC "${TRINEX_TINYGLTF_DIR}*.cpp")
set(TRINEX_ENGINE_SOURCE_FILES ${TRINEX_ENGINE_SOURCE_FILES} ${TRINEX_TINYGLTF_SRC})


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

if(UNIX)
    target_link_options(TrinexEngine PUBLIC -rdynamic)
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


## Include additional lib directory and include dir if build system is Windows OS
#if (WIN32)
#    target_link_directories(TrinexEngine "${TRINEX_ENGINE_ROOT}/libs/")
#endif()


# Linking libraries and including directories
target_include_directories(TrinexEngine PUBLIC "$<BUILD_INTERFACE:${TRINEX_ENGINE_ROOT}/include>")


target_link_libraries(TrinexEngine PRIVATE  ${PLATFORM_DEPENDED_LIBRARIES} ${PRIVATE_THIRD_PARTY_LIBRARIES})
target_link_libraries(TrinexEngine PUBLIC  ${PUBLIC_THIRD_PARTY_LIBRARIES})


install(TARGETS TrinexEngine DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
install(DIRECTORY configs DESTINATION ${TRINEX_CONFIGS_INSTALL_DIR})
install(DIRECTORY shaders DESTINATION ${TRINEX_SHADERS_INSTALL_DIR})

if (FALSE)
    # Compile glslang
    set(SPIRV_CROSS_STATIC ON)
    set(SPIRV_CROSS_SHARED OFF)
    set(SPIRV_CROSS_CLI OFF)
    set(SPIRV_CROSS_ENABLE_HLSL OFF)


    add_subdirectory(${TRINEX_ENGINE_ROOT}/libs/glslang/ EXCLUDE_FROM_ALL)
    add_subdirectory(${TRINEX_ENGINE_ROOT}/libs/SPIRV-Cross/ EXCLUDE_FROM_ALL)
    set_target_properties(spirv-cross-core PROPERTIES COMPILE_OPTIONS "-fPIC")
    set_target_properties(spirv-cross-glsl PROPERTIES COMPILE_OPTIONS "-fPIC")

    set(TRINEX_SHADER_COMPILERS_DIR "${TRINEX_ENGINE_SRC}/ShaderCompilers/")
    file(GLOB TRINEX_SHADER_COMPILERS_SRC "${TRINEX_SHADER_COMPILERS_DIR}*.cpp")
    add_library(ShaderCompilers SHARED ${TRINEX_SHADER_COMPILERS_SRC})
    target_link_libraries(ShaderCompilers PRIVATE TrinexEngine glslang SPIRV glslang-default-resource-limits spirv-cross-core spirv-cross-glsl)
    install(TARGETS ShaderCompilers DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
endif()

if (TRINEX_WITH_EDITOR)
    add_subdirectory(${TRINEX_ENGINE_ROOT}/editor)
endif()
