set(TRINEX_ENGINE_EDITOR_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SRC "${TRINEX_ENGINE_EDITOR_SOURCES_DIR}/*.cpp")

add_trinex_editor_sources(${TRINEX_ENGINE_EDITOR_SRC})
add_trinex_editor_libraries(assimp spirv-cross-glsl spirv-cross-core imgui imgui_node_editor imgui-filebrowser)

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

				if(TRINEX_ENGINE_EDITOR_LINK_DIRECTORIES)
								target_link_directories(TrinexEngineEditor PRIVATE ${TRINEX_ENGINE_EDITOR_LINK_DIRECTORIES})
				endif()
				
				if(TRINEX_ENGINE_EDITOR_INCLUDE_DIRECTORIES)
								target_include_directories(TrinexEngineEditor PRIVATE ${TRINEX_ENGINE_EDITOR_INCLUDE_DIRECTORIES})
				endif()
				
    target_link_libraries(TrinexEngineEditor PRIVATE TrinexEngine ${TRINEX_ENGINE_EDITOR_LIBRARIES})
    install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})
    
endif()

set_property(TARGET assimp PROPERTY COMPILE_OPTIONS "-g0" "-O3" "-Os" "-finline-functions" "-funroll-loops" "-fomit-frame-pointer")
