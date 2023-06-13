add_compile_options("-Wall")
add_compile_options("-Wno-unknown-pragmas")


if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options("-O3")
    add_compile_options("-Os")
else()
    add_compile_options("-g")
endif()
