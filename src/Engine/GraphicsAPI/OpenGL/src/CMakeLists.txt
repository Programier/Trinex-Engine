set(OPENGL_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB OPENGL_SRC "${OPENGL_SOURCES_DIR}/*.cpp")

if(WIN32)
    set(OPENGL_LIBS opengl32 glew_s)
elseif(ANDROID)
    set(OPENGL_LIBS GLESv3)
else()
    set(OPENGL_LIBS GL)
endif()


add_library(OpenGL SHARED ${OPENGL_SRC})
target_link_libraries(OpenGL PUBLIC ${OPENGL_LIBS} SDL2 TrinexEngine)

install (TARGETS OpenGL DESTINATION "./TrinexEngine/libs/")

target_compile_options(OpenGL PRIVATE "-Wall")
target_compile_options(OpenGL PRIVATE "-Wno-unknown-pragmas")
target_compile_options(OpenGL PRIVATE "-Wmismatched-tags")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(OpenGL PRIVATE "-O3")
    target_compile_options(OpenGL PRIVATE "-Os")
    target_compile_options(OpenGL PRIVATE "-finline-functions")
    target_compile_options(OpenGL PRIVATE "-funroll-loops")
    target_compile_options(OpenGL PRIVATE "-fomit-frame-pointer")
else()
    target_compile_options(OpenGL PRIVATE "-g")
endif()
