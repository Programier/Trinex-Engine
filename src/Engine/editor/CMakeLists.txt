cmake_minimum_required(VERSION 3.5)
project(TRINEX_ENGINE_EDITOR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TRINEX_ENGINE_EDITOR_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_EDITOR_SOURCES_ROOT ${TRINEX_ENGINE_EDITOR_PROJECT_ROOT}/src)


# PREPARE SLANG
include_directories(libs/slang/include/)

if(WIN32)
    link_directories("libs/slang/win64")
    install(FILES libs/slang/win64/slang-glslang.dll DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    install(FILES libs/slang/win64/slang.dll DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    set(SLANG_LIBRARIES slang slang-glslang)
elseif(LINUX)
    link_directories("libs/slang/linux-x86_64")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/linux-x86_64/libslang-glslang.so" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/linux-x86_64/libslang.so" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    set(SLANG_LIBRARIES slang slang-glslang)
endif()

# END PREPARE SLANG


add_subdirectory(libs)
add_subdirectory(src)
target_include_directories(TrinexEngineEditor PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})
