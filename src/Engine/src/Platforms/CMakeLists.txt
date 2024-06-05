add_subdirectory(Common)

if(WIN32)
    add_subdirectory(Windows)
elseif(LINUX)
    add_subdirectory(Linux)
elseif(ANDROID)
    add_subdirectory(Android)
endif()
