add_subdirectory("./libs/freetype/")
add_subdirectory("./libs/glfw3/")
add_subdirectory("./libs/glew/build/cmake")
include_directories("./libs/glew/include/")

if(WIN32)
    link_directories("./libs/")
    include_directories("./libs/include/")
	set(OPENGL opengl32)
	set(ASSIMP assimp-5)
else()
	set(OPENGL GL)
	set(ASSIMP assimp)
endif()

set(WINDOW_DIR "./Window/")
set(IMAGE_DIR "./Image/")
set(GRAPHICS_DIR "./Graphics/")
set(PHYSICS_DIR "./Physics/")

set(ENGINE Init.cpp
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
target_link_libraries(Engine glfw ${OPENGL} glew_s ${ASSIMP} freetype)
