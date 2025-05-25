file(GLOB D3D12_SRC "${D3D12_SOURCES_ROOT}/*.cpp")

add_trinex_sources(${D3D12_SRC})
add_trinex_private_libraries(d3d12.lib dxgi.lib)

if(WIN32)
    add_trinex_sources(${D3D12_SOURCES_ROOT}/platforms/windows.cpp)
endif()
