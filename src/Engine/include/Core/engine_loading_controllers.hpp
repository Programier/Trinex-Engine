#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <initializer_list>

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
        ControllerBase& push(const ControllerCallback& callback, const String& name = "",
                             const std::initializer_list<String>& require_initializers = {});
        ControllerBase& require(const String& name);
        ControllerBase& execute();
        virtual ~ControllerBase();
    };

#define IMPLEMENT_CONTROLLER(ControllerName)                                                                           \
    class ENGINE_EXPORT ControllerName : public ControllerBase                                                         \
    {                                                                                                                  \
    public:                                                                                                            \
        ControllerName();                                                                                              \
        ControllerName(const ControllerCallback& callback, const String& name = "",                                    \
                       const std::initializer_list<String>& require_initializers = {});                                \
    }

    IMPLEMENT_CONTROLLER(PostDestroyController);
    IMPLEMENT_CONTROLLER(DestroyController);
    IMPLEMENT_CONTROLLER(InitializeController);
    IMPLEMENT_CONTROLLER(PreInitializeController);


#undef IMPLEMENT_CONTROLLER
}// namespace Engine
