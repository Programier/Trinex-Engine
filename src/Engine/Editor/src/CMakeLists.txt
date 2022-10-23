file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(Editor ${SOURCES})
target_link_libraries(Editor Engine)
