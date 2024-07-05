#pragma once

#ifndef ENGINE_EXPORT

#ifdef _WIN32
    #define FORCE_ENGINE_EXPORT __declspec(dllexport)
    #define FORCE_ENGINE_IMPORT __declspec(dllimport)
#else
    #define FORCE_ENGINE_EXPORT __attribute__((visibility("default")))
    #define FORCE_ENGINE_IMPORT
#endif

#if defined( ENABLE_ENGINE_EXPORTS )
    #define ENGINE_EXPORT FORCE_ENGINE_EXPORT
#else
    #define ENGINE_EXPORT FORCE_ENGINE_IMPORT
#endif

#endif
