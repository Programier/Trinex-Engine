add_subdirectory(libs/vk-bootstrap)
add_subdirectory(libs/vulkan_memory_allocator)

add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/" "${CMAKE_CURRENT_SOURCE_DIR}/vulkan_headers/")

if(ANDROID)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^aarch64")
        add_trinex_link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/android/arm64-v8a/)
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        add_trinex_link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/android/x86_64/)
    endif()
endif()

add_subdirectory(src)
