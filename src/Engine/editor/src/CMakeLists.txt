set(TRINEX_ENGINE_EDITOR_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SRC "${TRINEX_ENGINE_EDITOR_SOURCES_DIR}/*.cpp")



if(TRINEX_REBUILD_HEADERS)
    set(EDITOR_PRECOMPILED_HEADERS DefaultResources/editor.hpp)
    add_custom_command(
        OUTPUT DefaultResources/editor.hpp
        COMMAND header_generator "${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/Editor.tpk" "${CMAKE_CURRENT_SOURCE_DIR}/DefaultResources/editor.hpp" "editor_package"
        COMMENT "Generating editor package header"
    )
endif()

add_executable(TrinexEngineEditor ${TRINEX_ENGINE_EDITOR_SRC} ${EDITOR_PRECOMPILED_HEADERS})

target_link_libraries(TrinexEngineEditor PUBLIC TrinexEngine    glslang
                                                                glslang-default-resource-limits
                                                                SPIRV
                                                                spirv-cross-core
                                                                spirv-cross-glsl
                                                                assimp
                                                                imgui_guizmo)

install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})
