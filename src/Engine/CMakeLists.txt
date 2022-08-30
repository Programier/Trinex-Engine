set(BUILD_SHARED_LIBS off)

project(Engine3D_RUN LANGUAGES CXX)

#Compile parameters
set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_SRC ${ENGINE_ROOT}/src)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory("${ENGINE_ROOT}/libs/freetype/" EXCLUDE_FROM_ALL)

if(NOT ANDROID)
    add_subdirectory("${ENGINE_ROOT}/libs/glew/build/cmake" EXCLUDE_FROM_ALL)
    include_directories("./libs/glew/include/")
endif()

#SDL
set(SDL_DIR "${ENGINE_ROOT}/libs/SDL")
add_subdirectory(${SDL_DIR})
set_target_properties(SDL2-static PROPERTIES COMPILE_OPTIONS "-fPIC")
include_directories("${SDL_DIR}/include")

#IMGUI
add_subdirectory("${ENGINE_ROOT}/libs/ImGUI")

#Assimp
#set(ENGINE_BUILD_ASSIMP on)


if(ENGINE_BUILD_ASSIMP)
    add_subdirectory("${ENGINE_ROOT}/libs/assimp" EXCLUDE_FROM_ALL)
    set(ASSIMP_LIB assimp)
else()
    CONFIGURE_FILE(
      ${CMAKE_CURRENT_LIST_DIR}/libs/assimp/include/assimp/config.h.in
      ${CMAKE_CURRENT_BINARY_DIR}/libs/assimp/include/assimp/config.h
    )
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/libs/assimp/include/")
endif()

include_directories("${ENGINE_ROOT}/libs/assimp/include/")
include_directories("${ENGINE_SRC}/EngineHeaders/")



if(WIN32)
    set(OPENGL opengl32 glew_s)
    set(WIN_NET ws2_32)
elseif(ANDROID)
    set(OPENGL EGL
               GLESv3)
else()
    set(OPENGL GL glew_s)
endif()

set(WINDOW_DIR "${ENGINE_SRC}/Window/")
set(IMAGE_DIR "${ENGINE_SRC}/Image/")
set(GRAPHICS_DIR "${ENGINE_SRC}/Graphics/")
set(PHYSICS_DIR "${ENGINE_SRC}/Physics/")
set(INTERNAL_DIR "${ENGINE_SRC}/InternalFunctions/")
set(NET_DIR "${ENGINE_SRC}/net/")
set(LIB_LOADER_DIR "${ENGINE_SRC}/LibLoader/")


# Find sources

file(GLOB WINDOW_SRC "${WINDOW_DIR}*.cpp")
file(GLOB IMAGE_SRC "${IMAGE_DIR}*.cpp")
file(GLOB GRAPHICS_SRC "${GRAPHICS_DIR}*.cpp")
file(GLOB PHYSICS_SRC "${PHYSICS_DIR}*.cpp")
file(GLOB INTERNAL_SRC "${INTERNAL_DIR}*.cpp")
file(GLOB NET_SRC "${NET_DIR}*.cpp")
file(GLOB LIB_LOADER_SRC "${LIB_LOADER_DIR}*.cpp")

set(ENGINE  ${WINDOW_SRC}
            ${IMAGE_SRC}
            ${GRAPHICS_SRC}
            ${PHYSICS_SRC}
            ${INTERNAL_SRC}
            #[[${NET_SRC}]]
            ${LIB_LOADER_SRC})

add_library(Engine SHARED ${ENGINE})
target_include_directories(Engine PUBLIC "$<BUILD_INTERFACE:${ENGINE_ROOT}/include>")
target_link_libraries(Engine SDL2-static ${OPENGL} ${ASSIMP_LIB} freetype ${WIN_NET} ImGUI)
