set(SOURCES main.cpp Application.cpp test.cpp GameInit.cpp)


add_executable(TrinexEngineLauncher ${SOURCES})

target_link_libraries(TrinexEngineLauncher PRIVATE TrinexEngine)
