cmake_minimum_required(VERSION 3.5)
project(TrinexEngine LANGUAGES C CXX ASM)

option(TRINEX_WITH_TRACY_PROFILER "Build Trinex Engine with supper Tracy Profiler" OFF)
option(TRINEX_WITH_VULKAN "Build Trinex Engine with support Vulkan API" ON)
option(TRINEX_WITH_OPENGL "Build Trinex Engine with support OpenGL API" ON)

if(WIN32)
    option(TRINEX_WITH_D3D11 "Build TrinexEngine with support DirectX 11 API" ON)
endif()

option(TRINEX_WITH_EDITOR "Build engine with editor" ON)
option(TRINEX_AS_EXECUTABLE "Compile trinex engine to executable file instead of shared library" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(TRINEX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_SRC "${TRINEX_ENGINE_ROOT}/src")

# Setup install directory
set(TRINEX_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/Engine" CACHE STRING "Install directory")
set(CMAKE_INSTALL_PREFIX ${TRINEX_INSTALL_DIR})

if(WIN32)
    set(TRINEX_LIBRARIES_INSTALL_DIR "${TRINEX_INSTALL_DIR}")
else()
    set(TRINEX_LIBRARIES_INSTALL_DIR "${TRINEX_INSTALL_DIR}/libs")
endif()

set(TRINEX_INCLUDES_INSTALL_DIR "${TRINEX_INSTALL_DIR}/include/")

if(UNIX)
    add_compile_options(-fvisibility=hidden)
endif()



######################## ENGINE COMPILATION FUNCTIONS ########################
set_property(GLOBAL PROPERTY trinex_private_libraries freetype asmjit zlib_library lz4_static)
set_property(GLOBAL PROPERTY trinex_private_headers_dir)
set_property(GLOBAL PROPERTY trinex_public_libraries fmt-header-only stb_library)
set_property(GLOBAL PROPERTY trinex_public_headers_dir ${TRINEX_ENGINE_ROOT}/include)
set_property(GLOBAL PROPERTY trinex_link_directories)
set_property(GLOBAL PROPERTY trinex_sources)
set_property(GLOBAL PROPERTY trinex_dependencies)
set_property(GLOBAL PROPERTY trinex_private_definitions -DENABLE_ENGINE_EXPORTS -DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES)
set_property(GLOBAL PROPERTY trinex_public_definitions)

function(add_trinex_sources sources)
    get_property(local_sources GLOBAL PROPERTY trinex_sources)
    set_property(GLOBAL PROPERTY trinex_sources ${local_sources} ${sources} ${ARGN})
endfunction()

function(add_trinex_link_directories directories)
    get_property(local_directories GLOBAL PROPERTY trinex_link_directories)
    set_property(GLOBAL PROPERTY trinex_link_directories ${local_directories} ${directories} ${ARGN})
endfunction()

function(add_trinex_public_include_directories headers)
    get_property(local_headers GLOBAL PROPERTY trinex_public_headers_dir)
    set_property(GLOBAL PROPERTY trinex_public_headers_dir ${local_headers} ${headers} ${ARGN})
endfunction()

function(add_trinex_public_libraries libraries)
    get_property(local_libraries GLOBAL PROPERTY trinex_public_libraries)
    set_property(GLOBAL PROPERTY trinex_public_libraries ${local_libraries} ${libraries} ${ARGN})
endfunction()

function(add_trinex_private_include_directories headers)
    get_property(local_headers GLOBAL PROPERTY trinex_private_headers_dir)
    set_property(GLOBAL PROPERTY trinex_private_headers_dir ${local_headers} ${headers} ${ARGN})
endfunction()

function(add_trinex_private_libraries libraries)
    get_property(local_libraries GLOBAL PROPERTY trinex_private_libraries)
    set_property(GLOBAL PROPERTY trinex_private_libraries ${local_libraries} ${libraries} ${ARGN})
endfunction()

function(add_trinex_dependencies dependencies)
    get_property(local_dependencies GLOBAL PROPERTY trinex_dependencies)
    set_property(GLOBAL PROPERTY trinex_dependencies ${local_dependencies} ${dependencies} ${ARGN})
endfunction()

function(add_trinex_public_definitions definitions)
    get_property(local_definitions GLOBAL PROPERTY trinex_public_definitions)
    set_property(GLOBAL PROPERTY trinex_public_definitions ${local_definitions} ${definitions} ${ARGN})
endfunction()

function(add_trinex_private_definitions definitions)
    get_property(local_definitions GLOBAL PROPERTY trinex_private_definitions)
    set_property(GLOBAL PROPERTY trinex_private_definitions ${local_definitions} ${definitions} ${ARGN})
endfunction()

######################## EXTERNAL LIBRARY COMPILATION ########################

# Compile Third-Party libraries
add_subdirectory("${TRINEX_ENGINE_ROOT}/libs/")

# If platform is Android, build android_native_app_glue library
if(ANDROID)
    set(NATIVE_APP_GLUE_SRC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
    include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)
    add_library(native_app_glue STATIC ${NATIVE_APP_GLUE_SRC})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
endif()


######################## ENGINE LIBRARY COMPILATION ########################

# List of libraries, which TrinexEngine use
if(ANDROID)
    find_library(glesv3-lib GLESv3)
    find_library(vulkan-lib vulkan)
    add_trinex_public_libraries(android native_app_glue log EGL ${glesv3-lib} ${vulkan-lib})
endif()


# Set source directories
set(TRINEX_SRC_WINDOW_DIR "${TRINEX_ENGINE_SRC}/Window/")
set(TRINEX_SRC_EVENT_DIR "${TRINEX_ENGINE_SRC}/Event/")
set(TRINEX_SRC_IMAGE_DIR "${TRINEX_ENGINE_SRC}/Image/")
set(TRINEX_SRC_GRAPHICS_DIR "${TRINEX_ENGINE_SRC}/Graphics/")
set(TRINEX_SRC_CORE_DIR "${TRINEX_ENGINE_SRC}/Core/")
set(TRINEX_SRC_NET_DIR "${TRINEX_ENGINE_SRC}/net/")
set(TRINEX_SRC_LIB_LOADER_DIR "${TRINEX_ENGINE_SRC}/LibLoader/")
set(TRINEX_SRC_SENSORS_DIR "${TRINEX_ENGINE_SRC}/Sensors/")
set(TRINEX_SRC_COMMANDLETS_DIR "${TRINEX_ENGINE_SRC}/CommandLets/")
set(TRINEX_SRC_SCRIPT_ENGINE_DIR "${TRINEX_ENGINE_SRC}/ScriptEngine/")
set(TRINEX_SRC_SYSTEMS_DIR "${TRINEX_ENGINE_SRC}/Systems/")
set(TRINEX_SRC_ENGINE_DIR "${TRINEX_ENGINE_SRC}/Engine/")
set(TRINEX_SRC_PLATFORM_DIR "${TRINEX_ENGINE_SRC}/Platforms/")
set(TRINEX_SRC_RHI_DIR "${TRINEX_ENGINE_SRC}/RHI/")
set(TRINEX_SRC_ENTRY_POINTS_DIR "${TRINEX_ENGINE_SRC}/EntryPoints/")


# Find sources
file(GLOB_RECURSE TRINEX_SRC_WINDOW_SRC "${TRINEX_SRC_WINDOW_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_IMAGE_SRC "${TRINEX_SRC_IMAGE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_GRAPHICS_SRC "${TRINEX_SRC_GRAPHICS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_CORE_SRC "${TRINEX_SRC_CORE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_NET_SRC "${TRINEX_SRC_NET_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_EVENT_SRC "${TRINEX_SRC_EVENT_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_SENSORS_SRC "${TRINEX_SRC_SENSORS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_COMMANDLETS_SRC "${TRINEX_SRC_COMMANDLETS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_SCRIPT_ENGINE_SRC "${TRINEX_SRC_SCRIPT_ENGINE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_SYSTEMS_SRC "${TRINEX_SRC_SYSTEMS_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_ENGINE_SRC "${TRINEX_SRC_ENGINE_DIR}*.cpp")
file(GLOB_RECURSE TRINEX_SRC_ENTRY_POINTS_SRC "${TRINEX_SRC_ENTRY_POINTS_DIR}*.cpp")

# Write all source files to one variable
add_trinex_sources( ${TRINEX_SRC_WINDOW_SRC}
                    ${TRINEX_SRC_IMAGE_SRC}
                    ${TRINEX_SRC_GRAPHICS_SRC}
                    ${TRINEX_SRC_CORE_SRC}
                    ${TRINEX_SRC_EVENT_SRC}
                    ${TRINEX_SRC_SENSORS_SRC}
                    ${TRINEX_SRC_COMMANDLETS_SRC}
                    ${TRINEX_SRC_SCRIPT_ENGINE_SRC}
                    ${TRINEX_SRC_SYSTEMS_SRC}
                    ${TRINEX_SRC_ENGINE_SRC}
                    ${TRINEX_SRC_PLATFORM_SRC}
                    ${TRINEX_SRC_ENTRY_POINTS_SRC})

# Collect platform source files
add_subdirectory(${TRINEX_SRC_PLATFORM_DIR})
add_subdirectory(${TRINEX_SRC_RHI_DIR})

# Collect sources of editor, if engine builds to executable file
if (TRINEX_WITH_EDITOR AND TRINEX_AS_EXECUTABLE)
    add_subdirectory(${TRINEX_ENGINE_ROOT}/editor)
endif()

# Compile Trinex Engine
get_property(TRINEX_ENGINE_SOURCE_FILES GLOBAL PROPERTY trinex_sources)

if(TRINEX_AS_EXECUTABLE)
    add_executable(TrinexEngine ${TRINEX_ENGINE_SOURCE_FILES})
else()
    add_library(TrinexEngine SHARED ${TRINEX_ENGINE_SOURCE_FILES})
endif()

get_property(TRINEX_ENGINE_DEPENDENCIES GLOBAL PROPERTY trinex_dependencies)
if(TRINEX_ENGINE_DEPENDENCIES)
    add_dependencies(TrinexEngine ${TRINEX_ENGINE_DEPENDENCIES})
endif()

# Trinex Engine compile options
target_compile_options(TrinexEngine PRIVATE "-Wall")
target_compile_options(TrinexEngine PRIVATE "-Wno-unknown-pragmas")
target_compile_options(TrinexEngine PRIVATE "-Wmismatched-tags")
target_compile_options(TrinexEngine PUBLIC "-fno-rtti")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(TrinexEngine PRIVATE "-O3")
    target_compile_options(TrinexEngine PRIVATE "-finline-functions")
    target_compile_options(TrinexEngine PRIVATE "-funroll-loops")
    target_compile_options(TrinexEngine PRIVATE "-fomit-frame-pointer")
else()
    target_compile_options(TrinexEngine PRIVATE "-g")
endif()

# Trinex Engine compile definitions
get_property(TRINEX_ENGINE_PUBLIC_DEFINITIONS GLOBAL PROPERTY trinex_public_definitions)
get_property(TRINEX_ENGINE_PRIVATE_DEFINITIONS GLOBAL PROPERTY trinex_private_definitions)

if(TRINEX_ENGINE_PUBLIC_DEFINITIONS)
    target_compile_definitions(TrinexEngine PUBLIC ${TRINEX_ENGINE_PUBLIC_DEFINITIONS})
endif()

if(TRINEX_ENGINE_PRIVATE_DEFINITIONS)
    target_compile_definitions(TrinexEngine PRIVATE ${TRINEX_ENGINE_PRIVATE_DEFINITIONS})
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_DEBUG_BUILD=1)
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_RELEASE_BUILD=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_DEBUG_BUILD=0)
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_RELEASE_BUILD=1)
endif()

# Linking libraries and including directories
get_property(TRINEX_ENGINE_PUBLIC_INCLUDE_DIRS GLOBAL PROPERTY trinex_public_headers_dir)
target_include_directories(TrinexEngine PUBLIC ${TRINEX_ENGINE_PUBLIC_INCLUDE_DIRS})

get_property(TRINEX_ENGINE_PRIVATE_INCLUDE_DIRS GLOBAL PROPERTY trinex_private_headers_dir)
if(TRINEX_ENGINE_PRIVATE_INCLUDE_DIRS)
    target_include_directories(TrinexEngine PRIVATE ${TRINEX_ENGINE_PRIVATE_INCLUDE_DIRS})
endif()

get_property(TRINEX_ENGINE_LINK_DIRECTORIES GLOBAL PROPERTY trinex_link_directories)
target_link_directories(TrinexEngine PUBLIC ${TRINEX_ENGINE_LINK_DIRECTORIES})

get_property(TRINEX_ENGINE_PRIVATE_LIBRARIES GLOBAL PROPERTY trinex_private_libraries)
target_link_libraries(TrinexEngine PRIVATE ${TRINEX_ENGINE_PRIVATE_LIBRARIES})
get_property(TRINEX_ENGINE_PUBLIC_LIBRARIES GLOBAL PROPERTY trinex_public_libraries)
target_link_libraries(TrinexEngine PUBLIC  ${TRINEX_ENGINE_PUBLIC_LIBRARIES})

if(TRINEX_AS_EXECUTABLE)
    install(TARGETS TrinexEngine DESTINATION ${TRINEX_INSTALL_DIR})
else()
    install(TARGETS TrinexEngine DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
endif()
install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})

if (TRINEX_WITH_EDITOR AND NOT TRINEX_AS_EXECUTABLE)
    add_subdirectory(${TRINEX_ENGINE_ROOT}/editor)
endif()

