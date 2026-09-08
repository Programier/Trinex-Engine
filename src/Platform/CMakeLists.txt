add_subdirectory(Common) 

add_subdirectory(SDL)

if (UNIX AND NOT ANDROID) 
    add_subdirectory(Linux) 
endif()
