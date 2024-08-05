if(WIN32)
    set(SLANG_LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win64")
else()
    set(SLANG_LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/linux-x86_64")
endif()

add_trinex_editor_link_directories(${SLANG_LIBRARIES_DIR})
add_trinex_editor_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


if(WIN32)
    install(FILES "${SLANG_LIBRARIES_DIR}/slang-glslang.dll" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    install(FILES "${SLANG_LIBRARIES_DIR}/slang.dll" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
elseif(LINUX)
    install(FILES "${SLANG_LIBRARIES_DIR}/libslang-glslang.so" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
    install(FILES "${SLANG_LIBRARIES_DIR}/libslang.so" DESTINATION ${TRINEX_LIBRARIES_INSTALL_DIR})
endif()
