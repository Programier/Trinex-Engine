include_directories("./libs/freetype/include/")
add_subdirectory("./libs/freetype/")
set(FREETYPE freetype)

include_directories("./libs/glfw/include/")
add_subdirectory("./libs/glfw3/")
set(GLFW glfw)

if(WIN32)
    link_directories("./libs/")
    include_directories("./libs/include/")
	set(OPENGL opengl32)
	set(GLEW_LIB glew32)
	set(ASSIMP assimp-5)
else()
	set(OPENGL GL)
	set(GLEW_LIB GLEW)
	set(ASSIMP assimp)
    include_directories("/usr/include/freetype2")
endif()

set(INIT_DIR "sources/Init/")
set(WINDOW_DIR "sources/Window/")
set(IMAGE_DIR "sources/Image/")
set(GRAPHICS_DIR "sources/Graphics/")
set(PHYSICS_DIR "sources/Physics/")

set(ENGINE ${INIT_DIR}/init.cpp
           ${WINDOW_DIR}/window.cpp
           ${WINDOW_DIR}/keyboard.cpp
           ${WINDOW_DIR}/color.cpp
           ${WINDOW_DIR}/cursor.cpp
           ${IMAGE_DIR}/image.cpp
           ${GRAPHICS_DIR}/shader.cpp
           ${GRAPHICS_DIR}/texture.cpp
           ${GRAPHICS_DIR}/camera.cpp
           ${GRAPHICS_DIR}/mesh.cpp
           ${GRAPHICS_DIR}/texturearray.cpp
           ${GRAPHICS_DIR}/basic_texturearray.cpp
           ${GRAPHICS_DIR}/model.cpp
           ${GRAPHICS_DIR}/skybox.cpp
           ${GRAPHICS_DIR}/line.cpp
           ${GRAPHICS_DIR}/text.cpp
           ${GRAPHICS_DIR}/heightmap.cpp
           ${PHYSICS_DIR}/terrain_collision.cpp)

add_library(Engine SHARED ${ENGINE})

include_directories("include/")
target_link_libraries(Engine ${GLFW} ${OPENGL} ${GLEW_LIB} ${ASSIMP} ${FREETYPE})
