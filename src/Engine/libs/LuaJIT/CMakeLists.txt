cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

set(LUAJIT_TOP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(luajit)

set(can_use_assembler TRUE)
enable_language(ASM)

include (GNUInstallDirs)


include_directories("${TRINEX_ENGINE_ROOT}/include/LuaJIT/")
if (WIN32)
    add_definitions("-DLUA_BUILD_AS_DLL")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options("-O3")
    add_compile_options("-Os")
else()
    add_compile_options("-g")
endif()

add_subdirectory(src)
