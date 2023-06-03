#pragma once

#ifndef ENGINE_EXPORT
#ifdef _WIN32
    #if defined( ENABLE_ENGINE_EXPORTS )
        #define ENGINE_EXPORT __declspec(dllexport)
    #else // !BUILDING_DLL
        #define ENGINE_EXPORT __declspec(dllimport)
    #endif // BUILDING_DLL
#else
    #if defined ( ENABLE_ENGINE_EXPORTS )
        #define ENGINE_EXPORT __attribute__((visibility("default")))
    #else
        #define ENGINE_EXPORT
    #endif
#endif // _WIN32

#endif
