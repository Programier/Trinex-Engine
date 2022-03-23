cmake_minimum_required(VERSION 3.5)
project(Engine3D_RUN LANGUAGES CXX)
add_compile_options(-Wall)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
file(COPY ${PROJECT_ROOT}/resources DESTINATION ${PROJECT_ROOT}/build/)
file(COPY ${PROJECT_ROOT}/Shaders DESTINATION ${PROJECT_ROOT}/build/)
include_directories("src/Engine/")
add_executable(Engine3D_RUN src/main.cpp)
add_subdirectory(src/Engine)
target_link_libraries(Engine3D_RUN Window Graphics)

