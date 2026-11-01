file(GLOB TRINEX_VULKAN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

find_package(Vulkan REQUIRED)

trinex_sources(TrinexEngine PRIVATE ${TRINEX_VULKAN_SOURCES})
trinex_link_libraries(TrinexEngine PRIVATE Vulkan::Vulkan)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    trinex_link_libraries(TrinexEngine PRIVATE VkLayer_khronos_validation)
endif()

if(ANDROID)
    trinex_sources(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android.cpp)
else()
    trinex_sources(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/SDL.cpp)
endif()

