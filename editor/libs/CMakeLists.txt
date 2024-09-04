if(UNIX)
    add_compile_options(-fvisibility=default)
endif()

add_subdirectory("assimp" EXCLUDE_FROM_ALL)
add_subdirectory("SPIRV-Cross" EXCLUDE_FROM_ALL)
add_subdirectory("slang")

add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(imgui-node-editor EXCLUDE_FROM_ALL)
add_subdirectory(imgui-filebrowser EXCLUDE_FROM_ALL)
add_subdirectory(ImGuiTextEditor EXCLUDE_FROM_ALL)
add_subdirectory("ImGuizmo" EXCLUDE_FROM_ALL)
