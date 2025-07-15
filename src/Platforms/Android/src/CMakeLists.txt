file(GLOB TRINEX_PLATFORM_ANDROID_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(TrinexEngine PRIVATE ${TRINEX_PLATFORM_ANDROID_SRC})
target_link_libraries(TrinexEngine PUBLIC dl)
