set(SOURCES main.cpp application_debug.cpp GUI.cpp Application.cpp)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Engine/libs/)

if(ANDROID)
    add_library(${OUTPUT_BIN} SHARED ${SOURCES})
else()
    add_executable(${OUTPUT_BIN} ${SOURCES})
endif()




target_link_libraries(${OUTPUT_BIN} PRIVATE Engine)
