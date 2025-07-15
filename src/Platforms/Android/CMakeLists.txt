target_include_directories(TrinexEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(src)

add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue/)
target_link_options(TrinexEngine PRIVATE "-u ANativeActivity_onCreate")
target_link_libraries(TrinexEngine PUBLIC android native_app_glue log EGL)
    
