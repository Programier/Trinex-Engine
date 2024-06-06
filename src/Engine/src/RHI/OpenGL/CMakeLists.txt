set(OPENGL_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(OPENGL_SOURCES_ROOT "${OPENGL_PROJECT_ROOT}/src/")

add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")

if(NOT ANDROID)
    add_trinex_private_include_directories("${OPENGL_PROJECT_ROOT}/libs/glew-2.2.0/include/")
    add_subdirectory("libs/glew-2.2.0/build/cmake/" EXCLUDE_FROM_ALL)
    set_target_properties(glew_s PROPERTIES COMPILE_OPTIONS "-fPIC")
    add_trinex_private_libraries(glew_s)
endif()

add_subdirectory(src)
