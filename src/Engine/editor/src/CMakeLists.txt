set(TRINEX_ENGINE_EDITOR_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SRC "${TRINEX_ENGINE_EDITOR_SOURCES_DIR}/*.cpp")



if(TRINEX_REBUILD_HEADERS)
    set(EDITOR_PRECOMPILED_HEADERS DefaultResources/editor.hpp)
    set(EDITOR_INTERNAL_ASSETS  ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/AddIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/AxisMaterial.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/DefaultIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/DefaultSampler.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/GridMaterial.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/MoveIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/RemoveIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/RotateIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/ScaleIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/SelectIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/PointLightSprite.tasset)
    add_custom_command(
        OUTPUT DefaultResources/editor.hpp
        COMMAND header_generator "${CMAKE_CURRENT_SOURCE_DIR}/DefaultResources/editor.hpp" ${EDITOR_INTERNAL_ASSETS}
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
