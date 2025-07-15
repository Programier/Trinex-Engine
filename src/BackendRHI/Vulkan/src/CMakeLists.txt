file(GLOB TRINEX_VULKAN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

find_package(Vulkan REQUIRED)

target_sources(TrinexEngine PRIVATE ${TRINEX_VULKAN_SOURCES})
target_link_libraries(TrinexEngine PRIVATE vk-bootstrap Vulkan::Vulkan)
target_include_directories(vk-bootstrap PRIVATE ${Vulkan_INCLUDE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(TrinexEngine PRIVATE VkLayer_khronos_validation)
endif()

if(ANDROID)
    target_sources(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android.cpp)
else()
    target_sources(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/common.cpp)
endif()

