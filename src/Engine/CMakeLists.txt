set(BUILD_SHARED_LIBS off)

add_subdirectory("./libs/freetype/" EXCLUDE_FROM_ALL)
add_subdirectory("./libs/glfw3/" EXCLUDE_FROM_ALL)
add_subdirectory("./libs/glew/build/cmake" EXCLUDE_FROM_ALL)
include_directories("./libs/glew/include/")
include_directories("./EngineHeaders/")

if(WIN32)
    set(LIBS_DIR "./libs/")
    link_directories(${LIBS_DIR})
    include_directories("./libs/include/")
    set(OPENGL opengl32)
    set(ASSIMP assimp-5)
    file(COPY ${LIBS_DIR}/lib${ASSIMP}.dll DESTINATION ${LIBRARY_OUTPUT_PATH})
    install(FILES ${LIBS_DIR}/lib${ASSIMP}.dll DESTINATION "./")
    set(WIN_NET ws2_32)
else()
    set(OPENGL GL)
    set(ASSIMP assimp)
endif()

set(WINDOW_DIR "./Window/")
set(IMAGE_DIR "./Image/")
set(GRAPHICS_DIR "./Graphics/")
set(PHYSICS_DIR "./Physics/")
set(INTERNAL_DIR "./InternalFunctions/")
set(NET_DIR "./net")

set(ENGINE
           ${WINDOW_DIR}/keyboard.cpp
           ${WINDOW_DIR}/cursor.cpp
           ${WINDOW_DIR}/window.cpp
           ${IMAGE_DIR}/image.cpp
           ${GRAPHICS_DIR}/shader.cpp
           ${GRAPHICS_DIR}/basic_framebuffer.cpp
           ${GRAPHICS_DIR}/basic_texture.cpp
           ${GRAPHICS_DIR}/enable_param.cpp
           ${GRAPHICS_DIR}/framebuffer.cpp
           ${GRAPHICS_DIR}/texture.cpp
           ${GRAPHICS_DIR}/camera.cpp
           ${GRAPHICS_DIR}/mesh.cpp
           ${GRAPHICS_DIR}/texturearray.cpp
           ${GRAPHICS_DIR}/basic_texturearray.cpp
           ${GRAPHICS_DIR}/terrainmodel.cpp
           ${GRAPHICS_DIR}/skybox.cpp
           ${GRAPHICS_DIR}/line.cpp
           ${GRAPHICS_DIR}/text.cpp
           ${GRAPHICS_DIR}/light.cpp
           ${GRAPHICS_DIR}/object.cpp
           ${GRAPHICS_DIR}/shader_system.cpp
           ${GRAPHICS_DIR}/basic_object.cpp
           ${GRAPHICS_DIR}/scene.cpp
           ${PHYSICS_DIR}/terrain_collision.cpp
           ${INTERNAL_DIR}/engine.cpp
           ${INTERNAL_DIR}/Init.cpp
           ${INTERNAL_DIR}/model_loader.cpp
           ${INTERNAL_DIR}/opengl.cpp
           #[[${NET_DIR}/server/server.cpp]])

add_library(Engine SHARED ${ENGINE})
include_directories("include/")
target_link_libraries(Engine glfw ${OPENGL} glew_s ${ASSIMP} freetype ${WIN_NET})
