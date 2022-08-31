#pragma once
#include <BasicFunctional/engine_types.hpp>

namespace Engine
{
    extern OpenGL_Version_S OpenGL_Ver;
    void init(const EngineAPI& API = EngineAPI::OpenGL);
    const EngineAPI& API();
    void except_init_check();
    bool is_inited();
}// namespace Engine
