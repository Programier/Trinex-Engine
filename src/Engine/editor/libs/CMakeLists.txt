add_subdirectory("glslang" EXCLUDE_FROM_ALL)
add_subdirectory("SPIRV-Cross" EXCLUDE_FROM_ALL)
add_subdirectory("assimp" EXCLUDE_FROM_ALL)
add_subdirectory("ImGuizmo" EXCLUDE_FROM_ALL)

set(PROJECTS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/ShaderWriter)
set(SDW_BUILD_STATIC ON)
add_subdirectory("ShaderWriter" EXCLUDE_FROM_ALL)
