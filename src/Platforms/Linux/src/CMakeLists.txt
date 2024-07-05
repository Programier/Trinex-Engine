set(PLATFORM_LINUX_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB PLATFORM_LINUX_SRC "${PLATFORM_LINUX_SOURCES_DIR}/*.cpp")

add_trinex_sources(${PLATFORM_LINUX_SRC})
add_trinex_private_libraries(dl)
