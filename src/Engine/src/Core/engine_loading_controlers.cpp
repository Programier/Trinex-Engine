#include <Core/demangle.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>

namespace Engine
{

    struct CallbackEntry {
        ControllerCallback function;
        Vector<String> require_initializers;
    };

    using CallbacksList      = Map<String, List<CallbackEntry>>;
    using CallbackListGetter = CallbacksList& (*) ();

    static CallbacksList& terminate_list()
    {
        static CallbacksList _M_terminate_list;
        return _M_terminate_list;
    }

    static CallbacksList& post_terminate_list()
    {
        static CallbacksList _M_terminate_list;
        return _M_terminate_list;
    }


    static CallbacksList& initialize_list()
    {
        static CallbacksList _M_init_list;
        return _M_init_list;
    }

    static CallbacksList& after_rhi_initialize_list()
    {
        static CallbacksList _M_init_list;
        return _M_init_list;
    }

    static CallbacksList& preinitialize_list()
    {
        static CallbacksList _M_init_list;
        return _M_init_list;
    }

    static CallbacksList& post_initialize_list()
    {
        static CallbacksList _M_init_list;
        return _M_init_list;
    }


    static inline CallbackListGetter convert_function_address(void* address)
    {
        return reinterpret_cast<CallbackListGetter>(address);
    }


    ControllerBase::ControllerBase(void* function_address, const char* name) : _M_func_address(function_address), _M_name(name)
    {}


    ControllerBase& ControllerBase::push(const ControllerCallback& callback, const String& name,
                                         const std::initializer_list<String>& require_initializers)
    {
        CallbackEntry entry;
        entry.function             = callback;
        entry.require_initializers = require_initializers;
        convert_function_address(_M_func_address)()[name].push_back(entry);
        return *this;
    }

    ControllerBase& ControllerBase::require(const String& name)
    {
        auto& initializers_list = convert_function_address(_M_func_address)();
        auto it                 = initializers_list.find(name);
        if (it == initializers_list.end())
            return *this;

        auto& list = it->second;

        while (!list.empty())
        {
            CallbackEntry entry = list.front();
            list.pop_front();

            for (auto& initializer : entry.require_initializers)
            {
                require(initializer);
            }

            if (!name.empty())
            {
                debug_log(_M_name, "Executing initializer '%s'", name.c_str());
            }
            entry.function();
        }

        return *this;
    }


    ControllerBase& ControllerBase::execute()
    {
        info_log(_M_name, "Executing command list!");

        auto& list = convert_function_address(_M_func_address)();

        while (!list.empty())
        {
            auto id = list.begin();
            require(id->first);
            list.erase(id);
        }

        return *this;
    }

    ControllerBase::~ControllerBase()
    {}


#define IMPLEMENT_CONTROLLER(ControllerName, func)                                                                               \
    ControllerName::ControllerName() : ControllerBase(reinterpret_cast<void*>(func), #ControllerName)                            \
    {}                                                                                                                           \
                                                                                                                                 \
                                                                                                                                 \
    ControllerName::ControllerName(const ControllerCallback& callback, const String& name,                                       \
                                   const std::initializer_list<String>& require_initializers)                                    \
        : ControllerName()                                                                                                       \
    {                                                                                                                            \
        push(callback, name, require_initializers);                                                                              \
    }                                                                                                                            \
    ControllerName& ControllerName::push(const ControllerCallback& callback, const String& name,                                 \
                                         const std::initializer_list<String>& require_initializers)                              \
    {                                                                                                                            \
        ControllerBase::push(callback, name, require_initializers);                                                              \
        return *this;                                                                                                            \
    }                                                                                                                            \
    ControllerName& ControllerName::require(const String& name)                                                                  \
    {                                                                                                                            \
        ControllerBase::require(name);                                                                                           \
        return *this;                                                                                                            \
    }                                                                                                                            \
    ControllerName& ControllerName::execute()                                                                                    \
    {                                                                                                                            \
        ControllerBase::execute();                                                                                               \
        return *this;                                                                                                            \
    }

    IMPLEMENT_CONTROLLER(PostDestroyController, post_terminate_list);
    IMPLEMENT_CONTROLLER(DestroyController, terminate_list);
    IMPLEMENT_CONTROLLER(InitializeController, initialize_list);
    IMPLEMENT_CONTROLLER(AfterRHIInitializeController, after_rhi_initialize_list);
    IMPLEMENT_CONTROLLER(PreInitializeController, preinitialize_list);
    IMPLEMENT_CONTROLLER(PostInitializeController, post_initialize_list);

}// namespace Engine
