file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
add_executable(TrinexEngineLauncher ${SOURCES})

link_libraries("/home/programier/Projects/Trinex-Engine/builds/build-Release-Linux/Game/libs/")
target_link_libraries(TrinexEngineLauncher PRIVATE TrinexEngine)
set_target_properties(TrinexEngineLauncher PROPERTIES LINK_FLAGS "-Wl,-rpath,./libs/")

