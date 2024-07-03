set(D3D11_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(D3D11_SOURCES_ROOT "${D3D11_PROJECT_ROOT}/src/")

add_trinex_private_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")
add_subdirectory(src)
