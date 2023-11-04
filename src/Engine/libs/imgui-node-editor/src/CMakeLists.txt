set(IMGUI_NODE_EDITOR_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB IMGUI_NODE_EDITOR_SRC "${IMGUI_NODE_EDITOR_SOURCES_DIR}/*.cpp")

add_definitions("-DIMGUI_NODE_EDITOR_EXPORT_ENABLE")

add_library(imgui_node_editor STATIC ${IMGUI_NODE_EDITOR_SRC})

include_directories("../../imgui/src/")
target_link_libraries(imgui_node_editor imgui)
set_target_properties(imgui_node_editor PROPERTIES COMPILE_OPTIONS "-fPIC")
