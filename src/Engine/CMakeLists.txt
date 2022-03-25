cmake_minimum_required(VERSION 3.5)
project(Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

if(WIN32)
    link_directories("./libs/")
    include_directories("./libs")
	set(OPENGL opengl32)
	set(GLEW_LIB glew32)
	set(ASSIMP assimp-5)
else()
	set(OPENGL GL)
	set(GLEW_LIB GLEW)
	set(ASSIMP assimp)
endif()

set(INIT_DIR "sources/Init/")
set(WINDOW_DIR "sources/Window/")
set(IMAGE_DIR "sources/Image/")
set(GRAPHICS_DIR "sources/Graphics/")

set(INIT ${INIT_DIR}/init.cpp)

set(WINDOW ${WINDOW_DIR}/window.cpp
           ${WINDOW_DIR}/keyboard.cpp
           ${WINDOW_DIR}/color.cpp
           ${WINDOW_DIR}/cursor.cpp)

set(IMAGE ${IMAGE_DIR}/image.cpp)

set(GRAPHICS ${GRAPHICS_DIR}/shader.cpp
             ${GRAPHICS_DIR}/texture.cpp
             ${GRAPHICS_DIR}/camera.cpp
             ${GRAPHICS_DIR}/mesh.cpp
             ${GRAPHICS_DIR}/texturearray.cpp
             ${GRAPHICS_DIR}/basic_texturearray.cpp
             ${GRAPHICS_DIR}/model.cpp
             ${GRAPHICS_DIR}/skybox.cpp)

add_library(Init SHARED ${INIT})
add_library(Window SHARED ${WINDOW})
add_library(Image SHARED ${IMAGE})
add_library(Graphics SHARED ${GRAPHICS})

include_directories("include/")

target_link_libraries(Init glfw ${OPENGL} ${GLEW_LIB})
target_link_libraries(Window Image Init)
target_link_libraries(Graphics Image Window Init ${ASSIMP})

