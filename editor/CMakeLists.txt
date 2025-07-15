file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(TRINEX_AS_EXECUTABLE)
    set(TRINEX_EDITOR_TARGET TrinexEngine)
    target_sources(TrinexEngine PRIVATE ${TRINEX_ENGINE_EDITOR_SOURCES})
else()
    set(TRINEX_EDITOR_TARGET TrinexEngineEditor)
    
    if(ANDROID)
        add_library(TrinexEngineEditor SHARED ${TRINEX_ENGINE_EDITOR_SOURCES})
    else()
        add_executable(TrinexEngineEditor PRIVATE ${TRINEX_ENGINE_EDITOR_SOURCES})
    endif()
    
    target_link_libraries(TrinexEngineEditor PRIVATE TrinexEngine)
    install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})
endif()

target_include_directories(${TRINEX_EDITOR_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(libs)
set_property(TARGET assimp PROPERTY COMPILE_OPTIONS "-O3" "-finline-functions" "-funroll-loops" "-fomit-frame-pointer")

target_link_libraries(${TRINEX_EDITOR_TARGET} PRIVATE assimp spirv-cross-glsl spirv-cross-core imgui imgui_node_editor imgui-filebrowser)
target_compile_definitions(${TRINEX_EDITOR_TARGET} PRIVATE "-DIMGUI_USER_CONFIG=<Core/imgui_config.hpp>")

install(DIRECTORY resources DESTINATION ${TRINEX_INSTALL_DIR})
