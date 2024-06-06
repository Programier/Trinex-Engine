set(VULKAN_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB VULKAN_SRC "${VULKAN_SOURCES_DIR}/*.cpp")

find_package(Vulkan REQUIRED)

add_trinex_sources(${VULKAN_SRC})
add_trinex_public_libraries(vk-bootstrap ${Vulkan_LIBRARY})

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ANDROID)
    add_trinex_public_libraries(VkLayer_khronos_validation)
endif()

