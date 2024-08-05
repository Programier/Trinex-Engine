set(TRINEX_ENGINE_EDITOR_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SRC "${TRINEX_ENGINE_EDITOR_SOURCES_DIR}/*.cpp")

add_trinex_editor_sources(${TRINEX_ENGINE_EDITOR_SRC})
add_trinex_editor_libraries(assimp spirv-cross-glsl spirv-cross-core)

if(NOT ANDROID)
    add_trinex_editor_libraries(slang)
endif()


if(TRINEX_REBUILD_HEADERS)
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
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/PointLightSprite.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/PointLightOverlay.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/SpotLightOverlay.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/MoreIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/BlueprintBackground.tasset)

    add_custom_target(
        TrinexEngineEditorHeaders ALL
        COMMAND header_generator "${CMAKE_CURRENT_SOURCE_DIR}/DefaultResources/editor.hpp" ${EDITOR_INTERNAL_ASSETS}
        COMMENT "Generating editor package header"
    )
    add_trinex_dependencies(TrinexEngineEditorHeaders)
endif()



get_property(TRINEX_ENGINE_EDITOR_SRC GLOBAL PROPERTY trinex_editor_sources)
get_property(TRINEX_ENGINE_EDITOR_LIBRARIES GLOBAL PROPERTY trinex_editor_libraries)
get_property(TRINEX_ENGINE_EDITOR_INCLUDE_DIRECTORIES GLOBAL PROPERTY trinex_editor_include_directories)
get_property(TRINEX_ENGINE_EDITOR_LINK_DIRECTORIES GLOBAL PROPERTY trinex_editor_link_directories)


if(TRINEX_AS_EXECUTABLE)
    add_trinex_sources(${TRINEX_ENGINE_EDITOR_SRC})
    add_trinex_private_libraries(${TRINEX_ENGINE_EDITOR_LIBRARIES})
    add_trinex_private_include_directories(${TRINEX_ENGINE_EDITOR_INCLUDE_DIRECTORIES})
    add_trinex_link_directories(${TRINEX_ENGINE_EDITOR_LINK_DIRECTORIES})
else()
    if(ANDROID)
        add_library(TrinexEngineEditor SHARED ${TRINEX_ENGINE_EDITOR_SRC})
    else()
        add_executable(TrinexEngineEditor ${TRINEX_ENGINE_EDITOR_SRC})
    endif()

    target_link_libraries(TrinexEngineEditor PRIVATE TrinexEngine ${TRINEX_ENGINE_EDITOR_LIBRARIES})
    install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})
    target_include_directories(TrinexEngineEditor PRIVATE ${TRINEX_ENGINE_EDITOR_INCLUDE_DIRECTORIES})
endif()

set_property(TARGET assimp PROPERTY COMPILE_OPTIONS "-g0" "-O3" "-Os" "-finline-functions" "-funroll-loops" "-fomit-frame-pointer")
