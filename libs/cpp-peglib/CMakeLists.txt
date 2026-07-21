cmake_minimum_required(VERSION 3.14)
project(peglib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /utf-8 /D_CRT_SECURE_NO_DEPRECATE")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(add_link_deps Threads::Threads)
endif()

add_library(peglib src/peglib.cpp)
target_include_directories(peglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(peglib PRIVATE cxx_std_20)

target_compile_definitions(peglib PRIVATE PEGLIB_EXPORTS)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(peglib PUBLIC PEGLIB_SHARED)
endif()

install(FILES include/peglib.h DESTINATION include)
install(TARGETS peglib)
