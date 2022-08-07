include_directories("${ENGINE_ROOT}/include/")
include_directories("${ENGINE_ROOT}/libs/glew/include/")
if (WIN32)
    include_directories("${ENGINE_ROOT}/libs/include/")
    link_directories("${ENGINE_ROOT}/libs")
endif()

set(EXECUTABLE_NAME ${PROJECT_NAME})

add_executable(${EXECUTABLE_NAME} main.cpp)
target_link_libraries(${EXECUTABLE_NAME} Engine)

install(TARGETS ${EXECUTABLE_NAME} Engine DESTINATION "./")
