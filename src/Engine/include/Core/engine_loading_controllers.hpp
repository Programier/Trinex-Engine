#pragma once
#include <Core/export.hpp>

namespace Engine
{
    class ENGINE_EXPORT DestroyController
    {
    public:
        DestroyController();
        DestroyController(void(*)());
        DestroyController& push(void(*)());
    };


    class ENGINE_EXPORT InitializeController
    {
    public:
        InitializeController();
        InitializeController(void(*)());
        InitializeController& push(void(*)());
    };

    class ENGINE_EXPORT PreInitializeController
    {
    public:
        PreInitializeController();
        PreInitializeController(void(*)());
        PreInitializeController& push(void(*)());
    };

}// namespace Engine
