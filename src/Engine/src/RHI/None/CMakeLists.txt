set(NONE_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(NONE_SOURCES_ROOT "${NONE_PROJECT_ROOT}/src/")

add_trinex_private_include_directories("${NONE_PROJECT_ROOT}/include/")
add_subdirectory(src)
