cmake_minimum_required(VERSION 3.5)

project(Engine3D_RUN LANGUAGES CXX)
add_compile_options(-Wall)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



include_directories("src/Engine/")
add_executable(Engine3D_RUN src/main.cpp)
add_subdirectory(src/Engine)
target_link_libraries(Engine3D_RUN Window Graphics)

