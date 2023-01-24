set(SOURCES main.cpp)

include_directories(${ENGINE_ROOT}/libs/assimp/include)
add_executable(AssimpViewer ${SOURCES})
target_link_libraries(AssimpViewer Engine)
