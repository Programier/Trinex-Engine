set(BUILD_SHARED_LIBS off)

# Compile AngelScript
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/angelscript/")

# Compile LZ4 library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lz4/build/cmake/")

# Compile freetype library
set(FT_DISABLE_HARFBUZZ on)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/freetype/" EXCLUDE_FROM_ALL)
set_target_properties(freetype PROPERTIES COMPILE_OPTIONS "-fPIC")

# Compile ImGUI
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/imgui/")

# Compile JSON library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/json/")

# Compile stb_image library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/stb_image")

# Compile tinygltf library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tinygltf")