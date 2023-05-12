cmake_minimum_required(VERSION 3.0)

project(minilua)

find_library(LIBM_LIBRARIES NAMES m)

add_executable(miniljs ../../miniljs.c)
if (LIBM_LIBRARIES)
  target_link_libraries(miniljs ${LIBM_LIBRARIES})
endif()

if (MSVC)
  set_target_properties(miniljs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
