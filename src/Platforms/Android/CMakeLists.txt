target_include_directories(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(src)

target_sources(TrinexEngine PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(TrinexEngine PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue/)
target_link_options(TrinexEngine PUBLIC "-u ANativeActivity_onCreate")
target_link_libraries(TrinexEngine PUBLIC android log EGL)
