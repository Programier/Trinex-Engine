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
        static CallbacksList m_terminate_list;
        return m_terminate_list;
    }

    static CallbacksList& post_terminate_list()
    {
        static CallbacksList m_terminate_list;
        return m_terminate_list;
    }


    static CallbacksList& initialize_list()
    {
        static CallbacksList m_init_list;
        return m_init_list;
    }

    static CallbacksList& after_rhi_initialize_list()
    {
        static CallbacksList m_init_list;
        return m_init_list;
    }

    static CallbacksList& preinitialize_list()
    {
        static CallbacksList m_init_list;
        return m_init_list;
    }

    static CallbacksList& post_initialize_list()
    {
        static CallbacksList m_init_list;
        return m_init_list;
    }

    static CallbacksList& class_initialize_list()
    {
        static CallbacksList m_init_list;
        return m_init_list;
    }

    static CallbacksList& default_resources_initialize_list()
    {
        static CallbacksList m_init_list;
        return m_init_list;
    }


    static inline CallbackListGetter convert_function_address(void* address)
    {
        return reinterpret_cast<CallbackListGetter>(address);
    }


    LoadingControllerBase::LoadingControllerBase(void* function_address, const char* name)
        : m_func_address(function_address), m_name(name)
    {}


    LoadingControllerBase& LoadingControllerBase::push(const ControllerCallback& callback, const String& name,
                                                       const std::initializer_list<String>& require_initializers)
    {
        CallbackEntry entry;
        entry.function             = callback;
        entry.require_initializers = require_initializers;
        convert_function_address(m_func_address)()[name].push_back(entry);
        return *this;
    }

    LoadingControllerBase& LoadingControllerBase::require(const String& name)
    {
        auto& initializers_list = convert_function_address(m_func_address)();
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
                debug_log(m_name, "Executing initializer '%s'", name.c_str());
            }
            entry.function();
        }

        return *this;
    }


    LoadingControllerBase& LoadingControllerBase::execute()
    {
        info_log(m_name, "Executing command list!");

        auto& list = convert_function_address(m_func_address)();

        while (!list.empty())
        {
            auto id = list.begin();
            require(id->first);
            list.erase(id);
        }

        return *this;
    }


    enum class ControllerType
    {
        PreInit  = BIT(0),
        Init     = BIT(1),
        PostInit = BIT(2),

        Destroy     = BIT(3),
        PostDestroy = BIT(4),

        ClassPreinit  = BIT(5),
        ClassInit     = BIT(6),
        ResourcesInit = BIT(7),
    };

    static Flags<ControllerType, BitMask> m_triggered;

    bool LoadingControllerBase::is_triggered(BitMask type)
    {
        return m_triggered & static_cast<ControllerType>(type);
    }

    void LoadingControllerBase::mark_triggered(BitMask type)
    {
        m_triggered |= static_cast<ControllerType>(type);
    }

    LoadingControllerBase::~LoadingControllerBase()
    {}


#define IMPLEMENT_CONTROLLER(ControllerName, func, type)                                                                         \
    ControllerName::ControllerName() : LoadingControllerBase(reinterpret_cast<void*>(func), #ControllerName)                     \
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
        LoadingControllerBase::push(callback, name, require_initializers);                                                       \
        return *this;                                                                                                            \
    }                                                                                                                            \
    ControllerName& ControllerName::require(const String& name)                                                                  \
    {                                                                                                                            \
        LoadingControllerBase::require(name);                                                                                    \
        return *this;                                                                                                            \
    }                                                                                                                            \
    ControllerName& ControllerName::execute()                                                                                    \
    {                                                                                                                            \
        LoadingControllerBase::execute();                                                                                        \
        LoadingControllerBase::mark_triggered(static_cast<BitMask>(ControllerType::type));                                       \
        return *this;                                                                                                            \
    }                                                                                                                            \
    bool ControllerName::is_triggered()                                                                                          \
    {                                                                                                                            \
        return LoadingControllerBase::is_triggered(static_cast<BitMask>(ControllerType::type));                                  \
    }

    IMPLEMENT_CONTROLLER(PreInitializeController, preinitialize_list, PreInit);
    IMPLEMENT_CONTROLLER(InitializeController, initialize_list, Init);
    IMPLEMENT_CONTROLLER(PostInitializeController, post_initialize_list, PostInit);

    IMPLEMENT_CONTROLLER(DestroyController, terminate_list, Destroy);
    IMPLEMENT_CONTROLLER(PostDestroyController, post_terminate_list, PostDestroy);


    IMPLEMENT_CONTROLLER(ClassInitializeController, class_initialize_list, ClassInit);
    IMPLEMENT_CONTROLLER(DefaultResourcesInitializeController, default_resources_initialize_list, ResourcesInit);

}// namespace Engine
