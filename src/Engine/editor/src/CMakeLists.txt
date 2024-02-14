set(TRINEX_ENGINE_EDITOR_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE TRINEX_ENGINE_EDITOR_SRC "${TRINEX_ENGINE_EDITOR_SOURCES_DIR}/*.cpp")


add_executable(TrinexEngineEditor ${TRINEX_ENGINE_EDITOR_SRC})

target_link_libraries(TrinexEngineEditor PUBLIC TrinexEngine glslang spirv-cross-core spirv-cross-glsl)

install(TARGETS TrinexEngineEditor DESTINATION ${TRINEX_INSTALL_DIR})
