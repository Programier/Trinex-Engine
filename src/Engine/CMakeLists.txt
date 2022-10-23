set(BUILD_SHARED_LIBS off)

project(GameEngine LANGUAGES CXX)
add_definitions(-DENABLE_ENGINE_EXPORTS)

#Compile parameters
set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_SRC "${ENGINE_ROOT}/src")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory("${ENGINE_ROOT}/libs/freetype/" EXCLUDE_FROM_ALL)
set_target_properties(freetype PROPERTIES COMPILE_OPTIONS "-fPIC")

#SDL
set(BUILD_SHARED_LIBS on)
set(SDL_SHARED on)
set(SDL_DIR "${ENGINE_ROOT}/libs/SDL")
add_subdirectory(${SDL_DIR} EXCLUDE_FROM_ALL)
include_directories("${SDL_DIR}/include")
set(BUILD_SHARED_LIBS off)

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
    set(OPENGL opengl32 glew)
elseif(ANDROID)
    set(OPENGL GLESv3)
else()
    set(OPENGL GL)
endif()

set(WINDOW_DIR "${ENGINE_SRC}/Window/")
set(EVENT_DIR "${ENGINE_SRC}/Event/")
set(IMAGE_DIR "${ENGINE_SRC}/Image/")
set(GRAPHICS_DIR "${ENGINE_SRC}/Graphics/")
set(PHYSICS_DIR "${ENGINE_SRC}/Physics/")
set(CORE_DIR "${ENGINE_SRC}/Core/")
set(NET_DIR "${ENGINE_SRC}/net/")
set(LIB_LOADER_DIR "${ENGINE_SRC}/LibLoader/")
set(IMGUI_DIR "${ENGINE_SRC}/ImGui/")
set(SENSORS_DIR "${ENGINE_SRC}/Sensors/")

# Find sources

file(GLOB WINDOW_SRC "${WINDOW_DIR}*.cpp")
file(GLOB IMAGE_SRC "${IMAGE_DIR}*.cpp")
file(GLOB GRAPHICS_SRC "${GRAPHICS_DIR}*.cpp")
file(GLOB PHYSICS_SRC "${PHYSICS_DIR}*.cpp")
file(GLOB CORE_SRC "${CORE_DIR}*.cpp")
file(GLOB NET_SRC "${NET_DIR}*.cpp")
file(GLOB LIB_LOADER_SRC "${LIB_LOADER_DIR}*.cpp")
file(GLOB IMGUI_SRC "${IMGUI_DIR}*.cpp")
file(GLOB EVENT_SRC "${EVENT_DIR}*.cpp")
file(GLOB SENSORS_SRC "${SENSORS_DIR}*.cpp")

set(ENGINE_SOURCE_FILES  ${WINDOW_SRC}
            ${IMAGE_SRC}
            ${GRAPHICS_SRC}
            ${PHYSICS_SRC}
            ${CORE_SRC}
            #[[${NET_SRC}]]
            ${LIB_LOADER_SRC}
            ${IMGUI_SRC}
            ${EVENT_SRC}
            ${SENSORS_SRC})



# Include private headers for imgui
include_directories(${ENGINE_SRC}/EngineHeaders/ImGui-Private/)

if (WIN32)
    include_directories("${ENGINE_ROOT}/libs/include/")
    link_directories("${ENGINE_ROOT}/libs/")
endif()

#if(BUILD_OPENGL)
add_subdirectory("${ENGINE_SRC}/OpenGL")
#endif()

set(BUILD_SHARED_LIBS on)

add_library(Engine SHARED ${ENGINE_SOURCE_FILES})
target_include_directories(Engine PUBLIC "$<BUILD_INTERFACE:${ENGINE_ROOT}/include>"
                                         "$<BUILD_INTERFACE:${ENGINE_ROOT}/include/ImGui>")

target_link_libraries(Engine PRIVATE SDL2 ${OPENGL} ${ASSIMP_LIB} freetype ${PlatformLibs})

add_subdirectory(Editor)
