file(GLOB_RECURSE TRINEX_PLATFORM_COMMON_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

        target_include_directories(TrinexEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
                target_sources(TrinexEngine PRIVATE ${TRINEX_PLATFORM_COMMON_SOURCES})
