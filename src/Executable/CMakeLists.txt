file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")


add_executable(TrinexEngineLauncher ${SOURCES})
target_link_libraries(TrinexEngineLauncher PRIVATE TrinexEngine)

if(ANDROID)
    add_library(TrinexEngineAndroidLauncher SHARED ${SOURCES})
    target_link_libraries(TrinexEngineAndroidLauncher PUBLIC TrinexEngine)
    target_include_directories(TrinexEngineAndroidLauncher PUBLIC "$<BUILD_INTERFACE:${GAME_PROJECT_ROOT}/include>")
    install(TARGETS TrinexEngineAndroidLauncher DESTINATION "./")
endif()

