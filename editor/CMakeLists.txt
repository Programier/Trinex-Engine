cmake_minimum_required(VERSION 3.5)
project(TRINEX_ENGINE_EDITOR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TRINEX_ENGINE_EDITOR_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_EDITOR_SOURCES_ROOT ${TRINEX_ENGINE_EDITOR_PROJECT_ROOT}/src)

if(WIN32)
    set(SLANG_LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/win64")
else()
    set(SLANG_LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/linux-x86_64")
endif()

# PREPARE SLANG
if(TRINEX_AS_EXECUTABLE)
    add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")
    add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/include/")
    add_trinex_link_directories(${SLANG_LIBRARIES_DIR})
else()
    include_directories(libs/slang/include/)
    link_directories(${SLANG_LIBRARIES_DIR})
endif()

if(WIN32)
    install(FILES "${SLANG_LIBRARIES_DIR}/slang-glslang.dll" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    install(FILES "${SLANG_LIBRARIES_DIR}/slang.dll" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
elseif(LINUX)
    install(FILES "${SLANG_LIBRARIES_DIR}/libslang-glslang.so" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    install(FILES "${SLANG_LIBRARIES_DIR}/libslang.so" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
endif()

# END PREPARE SLANG

add_subdirectory(libs)
add_subdirectory(src)


if(NOT TRINEX_AS_EXECUTABLE)
    target_include_directories(TrinexEngineEditor PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
endif()

install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})
