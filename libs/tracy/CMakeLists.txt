target_sources(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TracyClient.cpp)
target_include_directories(TrinexEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(TrinexEngine PRIVATE "-DTRACY_EXPORTS")
target_compile_definitions(TrinexEngine PUBLIC "-DTRACY_ENABLE")
