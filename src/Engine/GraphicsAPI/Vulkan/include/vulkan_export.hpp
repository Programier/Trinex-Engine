#pragma once


#ifdef _WIN32
    #if defined( ENABLE_VULKAN_EXPORTS )
        #define VULKAN_EXPORT __declspec(dllexport) __cdecl
    #else // !BUILDING_DLL
        #define VULKAN_EXPORT __declspec(dllimport)
    #endif // BUILDING_DLL
#else
    #define VULKAN_EXPORT
#endif // _WIN32

#define API_EXPORT extern "C" VULKAN_EXPORT
