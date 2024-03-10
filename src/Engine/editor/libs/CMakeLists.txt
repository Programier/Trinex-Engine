if(UNIX)
    add_compile_options(-fvisibility=default)
endif()

add_subdirectory("assimp" EXCLUDE_FROM_ALL)
add_subdirectory("ImGuizmo" EXCLUDE_FROM_ALL)
add_subdirectory("slang" EXCLUDE_FROM_ALL)
