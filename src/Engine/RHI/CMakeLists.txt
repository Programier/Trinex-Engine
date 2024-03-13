
# Compile OpenGL library
if(TRINEX_WITH_OPENGL)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/OpenGL")
endif()

# Compile Vulkan library
if(TRINEX_WITH_VULKAN)
    #add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Vulkan")
endif()
