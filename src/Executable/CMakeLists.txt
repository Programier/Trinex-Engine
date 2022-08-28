include_directories("${ENGINE_ROOT}/include/")
include_directories("${ENGINE_ROOT}/libs/glew/include/")
if (WIN32)
    include_directories("${ENGINE_ROOT}/libs/include/")
    link_directories("${ENGINE_ROOT}/libs")
endif()



add_executable(${OUTPUT_BIN} main.cpp application_debug.cpp GUI.cpp Application.cpp)
target_link_libraries(${OUTPUT_BIN} Engine ImGUI)

install(TARGETS ${OUTPUT_BIN} Engine DESTINATION "./")
