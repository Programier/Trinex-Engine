set(VULKAN_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB VULKAN_SRC "${VULKAN_SOURCES_DIR}/*.cpp")

find_package(Vulkan REQUIRED)

add_trinex_sources(${VULKAN_SRC})
add_trinex_public_libraries(vk-bootstrap ${Vulkan_LIBRARY})
target_include_directories(vk-bootstrap PRIVATE ${Vulkan_INCLUDE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_trinex_public_libraries(VkLayer_khronos_validation)
endif()

if(ANDROID)
    add_trinex_sources("${CMAKE_CURRENT_SOURCE_DIR}/platforms/android.cpp")
else()
    add_trinex_sources("${CMAKE_CURRENT_SOURCE_DIR}/platforms/common.cpp")
endif()

