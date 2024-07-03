file(GLOB D3D11_SRC "${D3D11_SOURCES_ROOT}/*.cpp")

add_trinex_sources(${D3D11_SRC})
add_trinex_private_libraries(d3d11)
