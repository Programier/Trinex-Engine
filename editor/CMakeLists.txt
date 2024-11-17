cmake_minimum_required(VERSION 3.5)
project(TRINEX_ENGINE_EDITOR LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TRINEX_ENGINE_EDITOR_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(TRINEX_ENGINE_EDITOR_SOURCES_ROOT ${TRINEX_ENGINE_EDITOR_PROJECT_ROOT}/src)

set_property(GLOBAL PROPERTY trinex_editor_libraries)
set_property(GLOBAL PROPERTY trinex_editor_sources)
set_property(GLOBAL PROPERTY trinex_editor_include_directories)
set_property(GLOBAL PROPERTY trinex_editor_link_directories)
set_property(GLOBAL PROPERTY trinex_editor_definitions)

function(add_trinex_editor_libraries libraries)
    get_property(local_libraries GLOBAL PROPERTY trinex_editor_libraries)
    set_property(GLOBAL PROPERTY trinex_editor_libraries ${local_libraries} ${libraries} ${ARGN})
endfunction()

function(add_trinex_editor_sources sources)
    get_property(local_sources GLOBAL PROPERTY trinex_editor_sources)
    set_property(GLOBAL PROPERTY trinex_editor_sources ${local_sources} ${sources} ${ARGN})
endfunction()

function(add_trinex_editor_include_directories directories)
    get_property(local_directories GLOBAL PROPERTY trinex_editor_include_directories)
    set_property(GLOBAL PROPERTY trinex_editor_include_directories ${local_directories} ${directories} ${ARGN})
endfunction()

function(add_trinex_editor_link_directories directories)
    get_property(local_directories GLOBAL PROPERTY trinex_editor_link_directories)
    set_property(GLOBAL PROPERTY trinex_editor_link_directories ${local_directories} ${directories} ${ARGN})
endfunction()

function(add_trinex_editor_definition definition)
    get_property(local_definitions GLOBAL PROPERTY trinex_editor_definitions)
    set_property(GLOBAL PROPERTY trinex_editor_definitions ${local_definitions} ${definition} ${ARGN})
endfunction()

add_trinex_editor_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_subdirectory(libs)
add_subdirectory(src)

install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})
