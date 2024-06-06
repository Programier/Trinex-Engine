set(OPENGL_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB OPENGL_SRC "${OPENGL_SOURCES_DIR}/*.cpp")

add_trinex_sources(${OPENGL_SRC})

if(WIN32)
    add_trinex_private_libraries(opengl32)
    add_trinex_private_libraries(glew_s)
elseif(ANDROID)
    add_trinex_private_libraries(GLESv3)
elseif(LINUX)
    add_trinex_private_libraries(GL)
    add_trinex_private_libraries(glew_s)
endif()
