#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{ 
    CLASS Init
    {
    public:
        void operator()(const EngineAPI& API = EngineAPI::OpenGL);
    };

    extern ENGINE_EXPORT OpenGL_Version_S OpenGL_Ver;
    extern ENGINE_EXPORT Init init;
    ENGINE_EXPORT void terminate();

    ENGINE_EXPORT const EngineAPI& Engine_API();
    ENGINE_EXPORT void except_init_check();
    ENGINE_EXPORT bool is_inited();
}// namespace Engine
