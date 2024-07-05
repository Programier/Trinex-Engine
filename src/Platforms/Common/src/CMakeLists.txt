if(WIN32 OR LINUX)
    set(PLATFORM_COMMON_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    file(GLOB PLATFORM_COMMON_SRC "${PLATFORM_COMMON_SOURCES_DIR}/*.cpp")

    add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include/")
    add_trinex_sources(${PLATFORM_COMMON_SRC})
    add_trinex_private_libraries(SDL2-static)
endif()
