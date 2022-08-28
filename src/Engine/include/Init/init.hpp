#pragma once
#include <BasicFunctional/engine_types.hpp>

namespace Engine
{
    namespace Monitor
    {
        void* monitor();
        int red_bits();
        int green_bits();
        int blue_bits();
        Size1D height();
        Size1D width();
        int refresh_rate();
        Size2D size();
        void update();
    }// namespace

    extern OpenGL_Version_S OpenGL_Ver;
    void init(const EngineAPI& API = EngineAPI::OpenGL);
    const EngineAPI& API();
    void except_init_check();
    bool is_inited();
}// namespace Engine
