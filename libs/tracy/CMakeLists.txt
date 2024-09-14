add_trinex_sources(${CMAKE_CURRENT_SOURCE_DIR}/TracyClient.cpp)

add_trinex_public_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_trinex_public_definitions("TRACY_ENABLE")
add_trinex_private_definitions("TRACY_EXPORTS")
