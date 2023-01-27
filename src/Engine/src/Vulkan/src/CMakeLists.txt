set(VULKAN_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB VULKAN_SRC "${VULKAN_SOURCES_DIR}/*.cpp")



add_library(Vulkan SHARED ${VULKAN_SRC})
target_link_libraries(Vulkan PUBLIC SDL2 vulkan)
