
if(WIN32)
    set(SLANG_LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win-${CMAKE_SYSTEM_PROCESSOR}")
elseif(LINUX)
    set(SLANG_LIBRARIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/linux-${CMAKE_SYSTEM_PROCESSOR}")
endif()

if(SLANG_LIBRARIES_DIR)
    add_trinex_editor_link_directories(${SLANG_LIBRARIES_DIR})
    add_trinex_editor_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_trinex_editor_libraries(slang)
    install(DIRECTORY "${SLANG_LIBRARIES_DIR}/" DESTINATION "${TRINEX_LIBRARIES_INSTALL_DIR}" FILES_MATCHING PATTERN "*.*")
endif()
