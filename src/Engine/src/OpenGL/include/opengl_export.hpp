#pragma once


#ifdef _WIN32
    #if defined( ENABLE_OPENGL_EXPORTS )
        #define OPENGL_EXPORT __declspec(dllexport) __cdecl
    #else // !BUILDING_DLL
        #define OPENGL_EXPORT __declspec(dllimport)
    #endif // BUILDING_DLL
#else
    #define OPENGL_EXPORT
#endif // _WIN32

#define API_EXPORT extern "C"  OPENGL_EXPORT
