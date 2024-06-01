set(VULKAN_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB VULKAN_SRC "${VULKAN_SOURCES_DIR}/*.cpp")


find_package(Vulkan REQUIRED)
set(VULKAN_LIBS Vulkan::Vulkan)

add_library(Vulkan SHARED ${VULKAN_SRC})
target_link_libraries(Vulkan PUBLIC ${VULKAN_LIBS} vk-bootstrap TrinexEngine)
install (TARGETS Vulkan DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})

target_compile_options(Vulkan PRIVATE "-Wall")
target_compile_options(Vulkan PRIVATE "-Wno-unknown-pragmas")
target_compile_options(Vulkan PRIVATE "-Wmismatched-tags")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(Vulkan PRIVATE "-O3")
    target_compile_options(Vulkan PRIVATE "-Os")
    target_compile_options(Vulkan PRIVATE "-finline-functions")
    target_compile_options(Vulkan PRIVATE "-funroll-loops")
    target_compile_options(Vulkan PRIVATE "-fomit-frame-pointer")
else()
    target_compile_options(Vulkan PRIVATE "-g")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Vulkan PRIVATE -DVULKAN_DEBUG_BUILD=1)
    target_compile_definitions(Vulkan PRIVATE -DVULKAN_RELEASE_BUILD=0)

    if(ANDROID)
        target_link_libraries(Vulkan PUBLIC VkLayer_khronos_validation)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Vulkan PRIVATE -DVULKAN_DEBUG_BUILD=0)
    target_compile_definitions(Vulkan PRIVATE -DVULKAN_RELEASE_BUILD=1)
endif()
