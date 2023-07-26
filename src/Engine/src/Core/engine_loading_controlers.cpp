#include <Core/demangle.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>

namespace Engine
{

    using CallbacksList      = List<ControllerCallback>;
    using CallbackListGetter = CallbacksList& (*) ();

    static CallbacksList& terminate_list()
    {
        static CallbacksList _M_terminate_list;
        return _M_terminate_list;
    }

    static CallbacksList& initialize_list()
    {
        static CallbacksList _M_init_list;
        return _M_init_list;
    }

    static CallbacksList& preinitialize_list()
    {
        static CallbacksList _M_init_list;
        return _M_init_list;
    }


    static inline CallbackListGetter convert_function_address(void* address)
    {
        return reinterpret_cast<CallbackListGetter>(address);
    }


    ControllerBase::ControllerBase(void* function_address) : _M_func_address(function_address)
    {}

    ControllerBase& ControllerBase::push(void (*callback)())
    {
        convert_function_address(_M_func_address)().push_back(callback);
        return *this;
    }

    ControllerBase& ControllerBase::execute()
    {
        auto name = Demangle::decode_name(typeid(*this));
        info_log(name.c_str(), "Executing command list!");

        auto& list = convert_function_address(_M_func_address)();

        while (!list.empty())
        {
            ControllerCallback func = list.front();
            list.pop_front();

            func();
        }

        return *this;
    }


#define IMPLEMENT_CONTROLLER(ControllerName, func)                                                                     \
    ControllerName::ControllerName() : ControllerBase(reinterpret_cast<void*>(func))                                   \
    {}                                                                                                                 \
                                                                                                                       \
    ControllerName::ControllerName(void (*callback)()) : ControllerName()                                              \
    {                                                                                                                  \
        push(callback);                                                                                                \
    }


    IMPLEMENT_CONTROLLER(DestroyController, terminate_list);
    IMPLEMENT_CONTROLLER(InitializeController, initialize_list);
    IMPLEMENT_CONTROLLER(PreInitializeController, preinitialize_list);

}// namespace Engine
