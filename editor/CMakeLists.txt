file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(TRINEX_AS_EXECUTABLE)
    set(TRINEX_EDITOR_TARGET TrinexEngine)
    trinex_sources(TrinexEngine PRIVATE ${TRINEX_ENGINE_EDITOR_SOURCES})
else()
    set(TRINEX_EDITOR_TARGET TrinexEngineEditor)
    
    if(ANDROID)
        trinex_library(TrinexEngineEditor SHARED ${TRINEX_ENGINE_EDITOR_SOURCES})
    else()
        trinex_executable(TrinexEngineEditor ${TRINEX_ENGINE_EDITOR_SOURCES})
    endif()
    
    trinex_link_libraries(TrinexEngineEditor PRIVATE TrinexEngine)
    trinex_install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})
endif()

trinex_reflection_directories(${TRINEX_EDITOR_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(libs)
set_property(TARGET assimp PROPERTY COMPILE_OPTIONS "-O3" "-finline-functions" "-funroll-loops" "-fomit-frame-pointer")

trinex_link_libraries(${TRINEX_EDITOR_TARGET} PRIVATE assimp spirv-cross-glsl spirv-cross-core imgui imgui_node_editor imgui-filebrowser)
trinex_compile_definitions(${TRINEX_EDITOR_TARGET} PRIVATE "-DIMGUI_USER_CONFIG=<Core/imgui_config.hpp>")

trinex_install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})
