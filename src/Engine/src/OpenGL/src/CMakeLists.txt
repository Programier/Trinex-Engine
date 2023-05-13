set(OPENGL_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB OPENGL_SRC "${OPENGL_SOURCES_DIR}/*.cpp")


if(WIN32)
    set(OPENGL_LIBS opengl32 glew)
elseif(ANDROID)
    set(OPENGL_LIBS GLESv3)
else()
    set(OPENGL_LIBS GL)
endif()


add_library(OpenGL SHARED ${OPENGL_SRC})
target_link_libraries(OpenGL PUBLIC ${OPENGL_LIBS} SDL2 TrinexEngine)

install (TARGETS OpenGL DESTINATION "./TrinexEngine/libs/")
