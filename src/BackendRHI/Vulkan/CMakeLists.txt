add_subdirectory(libs/vk-bootstrap)
add_subdirectory(libs/vulkan_memory_allocator)

target_include_directories(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_headers)

if(ANDROID)
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "^aarch64")
        target_link_directories(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/android/arm64-v8a/)
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        target_link_directories(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/android/x86_64/)
    endif()
endif()

add_subdirectory(src)
