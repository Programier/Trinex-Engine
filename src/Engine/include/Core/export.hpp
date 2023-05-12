#pragma once

#ifndef ENGINE_EXPORT
#ifdef _WIN32
    #if defined( ENABLE_ENGINE_EXPORTS )
        #define ENGINE_EXPORT __declspec(dllexport)
    #else // !BUILDING_DLL
        #define ENGINE_EXPORT __declspec(dllimport)
    #endif // BUILDING_DLL
#else
    #define ENGINE_EXPORT
#endif // _WIN32

#endif

#ifndef FORCE_EXPORT
#ifdef _WIN32
    #define FORCE_EXPORT __declspec(dllexport)
#else
    #define FORCE_EXPORT
#endif // _WIN32

#endif
