set(OPENGL_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB OPENGL_SRC "${OPENGL_SOURCES_DIR}/*.cpp")

if(WIN32)
    set(OPENGL opengl32 glew)
elseif(ANDROID)
    set(OPENGL GLESv3)
else()
    set(OPENGL GL)
endif()

add_library(OpenGL SHARED ${OPENGL_SRC})
target_link_libraries(OpenGL PRIVATE ${OPENGL} SDL2)
