# Compile NoneAPI library (Always compile)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/None/")

# Compile Vulkan library
if(TRINEX_WITH_VULKAN)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Vulkan")
endif()

# Compile DX11 library
if(WIN32 AND TRINEX_WITH_D3D12)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/D3D12/")
endif()
