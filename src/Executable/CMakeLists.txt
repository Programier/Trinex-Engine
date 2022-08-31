set(SOURCES main.cpp application_debug.cpp GUI.cpp Application.cpp)

if(ANDROID)
    add_library(${OUTPUT_BIN} SHARED ${SOURCES})
else()
    add_executable(${OUTPUT_BIN} ${SOURCES})
endif()

target_link_libraries(${OUTPUT_BIN} Engine)
install(TARGETS ${OUTPUT_BIN} Engine DESTINATION "./")
