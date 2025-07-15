file(GLOB TRINEX_D3D12_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(TrinexEngine PRIVATE ${TRINEX_D3D12_SOURCES})
target_link_libraries(TrinexEngine PRIVATE d3d12.lib dxgi.lib)

if(WIN32)
    target_sources(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows.cpp)
endif()
