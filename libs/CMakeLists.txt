set(BUILD_SHARED_LIBS off)

# Compile AngelScript
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/angelscript/")

# Compile LZ4 library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lz4/build/cmake/" EXCLUDE_FROM_ALL)

# Compile freetype library
set(FT_DISABLE_HARFBUZZ on)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/freetype/" EXCLUDE_FROM_ALL)
set_target_properties(freetype PROPERTIES COMPILE_OPTIONS "-fPIC")

# Compile stb library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/stb")

# Compile AsmJIT library
set(ASMJIT_STATIC on)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/asmjit/" EXCLUDE_FROM_ALL)
set_target_properties(asmjit PROPERTIES COMPILE_OPTIONS "-fPIC")

add_subdirectory(zlib EXCLUDE_FROM_ALL)

add_subdirectory(fmt EXCLUDE_FROM_ALL)
set_target_properties(fmt PROPERTIES COMPILE_OPTIONS "-fPIC")

if (WIN32 OR LINUX)
    add_subdirectory(SDL)
endif()

if (TRINEX_WITH_TRACY_PROFILER)
	add_subdirectory(tracy)
endif()

add_subdirectory(glm)


add_trinex_public_libraries(glm fmt-header-only)
