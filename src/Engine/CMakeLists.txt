cmake_minimum_required(VERSION 3.5)
project(Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)
set(INIT Init/init.cpp)
set(WINDOW Window/window.cpp Window/keyboard.cpp Window/color.cpp Window/cursor.cpp)
set(IMAGE Image/image.cpp)
set(GRAPHICS Graphics/shader.cpp
             Graphics/texture.cpp
             Graphics/camera.cpp
             Graphics/mesh.cpp
             Graphics/texturearray.cpp
             Graphics/basic_texturearray.cpp
             Graphics/model.cpp
             Graphics/skybox.cpp)

add_library(Init SHARED ${INIT})
add_library(Window SHARED ${WINDOW})
add_library(Image SHARED ${IMAGE})
add_library(Graphics SHARED ${GRAPHICS})

target_link_libraries(Init glfw GL GLEW)
target_link_libraries(Window Image Init)
target_link_libraries(Graphics Init assimp)

