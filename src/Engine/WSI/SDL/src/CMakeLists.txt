set(SDL_SYSTEM_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB SDL_SYSTEM_SRC "${SDL_SYSTEM_SOURCES_DIR}/*.cpp")

add_library(WindowSystemSDL2 SHARED ${SDL_SYSTEM_SRC})
target_link_libraries(WindowSystemSDL2 PUBLIC SDL2 TrinexEngine)

install (TARGETS WindowSystemSDL2 SDL2 DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})

target_compile_options(WindowSystemSDL2 PRIVATE "-Wall")
target_compile_options(WindowSystemSDL2 PRIVATE "-Wno-unknown-pragmas")
target_compile_options(WindowSystemSDL2 PRIVATE "-Wmismatched-tags")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../libs/SDL/include/")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(WindowSystemSDL2 PRIVATE "-O3")
    target_compile_options(WindowSystemSDL2 PRIVATE "-Os")
    target_compile_options(WindowSystemSDL2 PRIVATE "-finline-functions")
    target_compile_options(WindowSystemSDL2 PRIVATE "-funroll-loops")
    target_compile_options(WindowSystemSDL2 PRIVATE "-fomit-frame-pointer")
else()
    target_compile_options(WindowSystemSDL2 PRIVATE "-g")
endif()

if(UNIX)
    target_link_options(WindowSystemSDL2 PUBLIC -rdynamic)
endif()
