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

add_subdirectory(fmt EXCLUDE_FROM_ALL)
set_target_properties(fmt PROPERTIES COMPILE_OPTIONS "-fPIC")

if (TRINEX_WITH_TRACY_PROFILER)
    add_subdirectory(tracy)
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tlsf")

# mimalloc
set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MI_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MI_OVERRIDE OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/mimalloc" EXCLUDE_FROM_ALL)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/cpp-peglib")
set_target_properties(peglib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(TrinexEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glm-lib)
target_include_directories(TrinexEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/json/include)


# link
trinex_link_libraries(TrinexEngine PRIVATE mimalloc-static)
trinex_link_libraries(TrinexEngine PUBLIC fmt-header-only)
trinex_link_libraries(TrinexEngine PUBLIC peglib)
