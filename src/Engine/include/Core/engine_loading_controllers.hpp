#pragma once
#include <Core/export.hpp>

namespace Engine
{
    class ENGINE_EXPORT DestroyController
    {
    public:
        DestroyController(void(*)());
    };


    class ENGINE_EXPORT InitializeController
    {
    public:
        InitializeController(void(*)());
    };

    class ENGINE_EXPORT PreInitializeController
    {
    public:
        PreInitializeController(void(*)());
    };

}// namespace Engine
