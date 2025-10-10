cmake_minimum_required(VERSION 3.20)
project(TrinexEngine LANGUAGES C CXX ASM)

option(TRINEX_WITH_TRACY_PROFILER "Build Trinex Engine with supper Tracy Profiler" OFF)
option(TRINEX_WITH_VULKAN "Build Trinex Engine with support Vulkan API" ON)

if(WIN32)
    option(TRINEX_WITH_D3D12 "Build TrinexEngine with support DirectX 12 API" ON)
endif()

option(TRINEX_WITH_EDITOR "Build engine with editor" ON)
option(TRINEX_WITH_ADDRESS_SANITIZER OFF)
option(TRINEX_AS_EXECUTABLE "Compile trinex engine to executable file instead of shared library" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(TRINEX_ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_SRC "${TRINEX_ENGINE_ROOT}/src")
set(TRINEX_ENGINE_INCLUDE "${TRINEX_ENGINE_ROOT}/include")

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

set(TRINEX_SOURCE_DIRECTORIES
    "${TRINEX_ENGINE_SRC}/Window/"
    "${TRINEX_ENGINE_SRC}/Event/"
    "${TRINEX_ENGINE_SRC}/Image/"
    "${TRINEX_ENGINE_SRC}/Graphics/"
    "${TRINEX_ENGINE_SRC}/Core/"
    "${TRINEX_ENGINE_SRC}/net/"
    "${TRINEX_ENGINE_SRC}/LibLoader/"
    "${TRINEX_ENGINE_SRC}/Sensors/"
    "${TRINEX_ENGINE_SRC}/CommandLets/"
    "${TRINEX_ENGINE_SRC}/ScriptEngine/"
    "${TRINEX_ENGINE_SRC}/Systems/"
    "${TRINEX_ENGINE_SRC}/Engine/"
    "${TRINEX_ENGINE_SRC}/RHI/"
    "${TRINEX_ENGINE_SRC}/EntryPoints/"
)

set(TRINEX_SOURCE_FILES)

foreach(dir ${TRINEX_SOURCE_DIRECTORIES})
    file(GLOB_RECURSE TRINEX_DIRECTORY_SOURCES CONFIGURE_DEPENDS ${dir}/*.cpp)
    list(APPEND TRINEX_SOURCE_FILES ${TRINEX_DIRECTORY_SOURCES})
endforeach()

# Compile Trinex Engine
if(TRINEX_AS_EXECUTABLE AND NOT ANDROID)
    add_executable(TrinexEngine ${TRINEX_SOURCE_FILES})
else()
    add_library(TrinexEngine SHARED ${TRINEX_SOURCE_FILES})
endif()

# Compile modules
add_subdirectory(${TRINEX_ENGINE_ROOT}/libs/)
add_subdirectory(${TRINEX_ENGINE_SRC}/Platforms)
add_subdirectory(${TRINEX_ENGINE_SRC}/BackendRHI)

if(TRINEX_WITH_EDITOR)
    add_subdirectory(${TRINEX_ENGINE_ROOT}/editor)
endif()

# Trinex Engine compile options
target_include_directories(TrinexEngine PUBLIC ${TRINEX_ENGINE_ROOT}/include)
target_link_libraries(TrinexEngine PUBLIC freetype asmjit zlib_library lz4_static stb_library)

target_compile_definitions(TrinexEngine PRIVATE -DENABLE_ENGINE_EXPORTS=1)

target_compile_options(TrinexEngine PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:
        $<$<CXX_COMPILER_ID:GNU,Clang>:-include ${TRINEX_ENGINE_INCLUDE}/Core/preincluded.hpp>
        $<$<CXX_COMPILER_ID:MSVC>:/FI${TRINEX_ENGINE_INCLUDE}/Core/preincluded.hpp>
    >
)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_DEBUG_BUILD=1)
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_RELEASE_BUILD=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_DEBUG_BUILD=0)
    target_compile_definitions(TrinexEngine PRIVATE -DTRINEX_RELEASE_BUILD=1)
endif()

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
    
    if(TRINEX_WITH_ADDRESS_SANITIZER)
        target_compile_options(TrinexEngine PRIVATE 
            -fsanitize=address 
            -fno-omit-frame-pointer 
            -fno-sanitize=all 
        )
    
        target_link_options(TrinexEngine PRIVATE 
            -fsanitize=address 
            -Wl,-rpath,$ORIGIN
        )
    
        set_target_properties(TrinexEngine PROPERTIES
            LINK_DEPENDS_NO_SANITIZE 1
            LINK_FLAGS_RELEASE "-Wl,--no-as-needed"
        )
    endif()
endif()

if(TRINEX_AS_EXECUTABLE)
    install(TARGETS TrinexEngine DESTINATION ${TRINEX_INSTALL_DIR})
else()
    install(TARGETS TrinexEngine DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
endif()
install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})
