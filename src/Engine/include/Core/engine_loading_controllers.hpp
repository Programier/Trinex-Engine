#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{

    using ControllerCallback = Function<void()>;
    class ENGINE_EXPORT ControllerBase
    {
    private:
        void* _M_func_address = nullptr;

    protected:
        ControllerBase(void* function_address);

    public:
        ControllerBase& push(void (*)());
        ControllerBase& push(const ControllerCallback& callback);
        ControllerBase& execute();
    };

#define IMPLEMENT_CONTROLLER(ControllerName)                                                                           \
    class ENGINE_EXPORT ControllerName : public ControllerBase                                                         \
    {                                                                                                                  \
    public:                                                                                                            \
        ControllerName();                                                                                              \
        ControllerName(void (*)());                                                                                    \
        ControllerName(const ControllerCallback& callback);                                                            \
    }

    IMPLEMENT_CONTROLLER(DestroyController);
    IMPLEMENT_CONTROLLER(InitializeController);
    IMPLEMENT_CONTROLLER(PreInitializeController);

#undef IMPLEMENT_CONTROLLER
}// namespace Engine
