
# Compile OpenGL library
if(TRINEX_WITH_OPENGL)
    #add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/OpenGL")
endif()

# Compile Vulkan library (Windows OS currently not supported)
if(WIN32)
    set(TRINEX_WITH_VULKAN off)
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Vulkan")
if(TRINEX_WITH_VULKAN)
endif()
