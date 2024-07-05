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
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/PointLightSprite.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/PointLightOverlay.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/SpotLightOverlay.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/MoreIcon.tasset
                                ${CMAKE_CURRENT_SOURCE_DIR}/../resources_internal/BlueprintBackground.tasset)
    add_custom_command(
        OUTPUT DefaultResources/editor.hpp
        COMMAND header_generator "${CMAKE_CURRENT_SOURCE_DIR}/DefaultResources/editor.hpp" ${EDITOR_INTERNAL_ASSETS}
        COMMENT "Generating editor package header"
    )
endif()


if(TRINEX_AS_EXECUTABLE)
    add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../libs/slang/source/core/")
    add_trinex_sources(${TRINEX_ENGINE_EDITOR_SRC} ${EDITOR_PRECOMPILED_HEADERS})
    add_trinex_private_libraries(assimp spirv-cross-glsl spirv-cross-core)

    if(NOT ANDROID)
        add_trinex_private_libraries(slang)
    endif()
else()
    include_directories(../libs/slang/source/core/)

    if(ANDROID)
        add_library(TrinexEngineEditor SHARED ${TRINEX_ENGINE_EDITOR_SRC} ${EDITOR_PRECOMPILED_HEADERS})
    else()
        add_executable(TrinexEngineEditor ${TRINEX_ENGINE_EDITOR_SRC} ${EDITOR_PRECOMPILED_HEADERS})
    endif()

    target_link_libraries(TrinexEngineEditor PRIVATE TrinexEngine   assimp
                                                                    imgui_guizmo
                                                                    spirv-cross-glsl
                                                                    spirv-cross-core)
    if(NOT ANDROID)
        target_link_libraries(TrinexEngineEditor PRIVATE slang)
    endif()

    install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})

    set_property(TARGET imgui_guizmo PROPERTY COMPILE_OPTIONS "-g0" "-O3" "-Os" "-finline-functions" "-funroll-loops" "-fomit-frame-pointer")
endif()

set_property(TARGET assimp PROPERTY COMPILE_OPTIONS "-g0" "-O3" "-Os" "-finline-functions" "-funroll-loops" "-fomit-frame-pointer")
