add_library(imgui_node_editor INTERFACE)

set(TRINEX_EXTERNAL_SOURCES ${TRINEX_EXTERNAL_SOURCES}
                            ${CMAKE_CURRENT_SOURCE_DIR}/crude_json.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/imgui_canvas.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/imgui_node_editor.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/imgui_node_editor_api.cpp PARENT_SCOPE)
