#pragma once


#ifdef _WIN32
    #if defined( ENABLE_ENGINE_EXPORTS )
        #define ENGINE_EXPORT __declspec(dllexport)
    #else // !BUILDING_DLL
        #define ENGINE_EXPORT __declspec(dllimport)
    #endif // BUILDING_DLL
#else
    #define ENGINE_EXPORT
#endif // _WIN32

#define CLASS class ENGINE_EXPORT
#define STRUCT struct ENGINE_EXPORT
