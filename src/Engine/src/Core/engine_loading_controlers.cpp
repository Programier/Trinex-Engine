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

    using CallbackListGetter = CallbacksList& (*) ();

    template<typename T>
    static CallbacksList& callbacks_list()
    {
        static CallbacksList list;
        return list;
    }

    LoadingControllerBase::LoadingControllerBase(CallbacksList& list, const char* name) : m_list(list), m_name(name)
    {}


    void LoadingControllerBase::exec_all_if_already_triggered()
    {
        if (PreInitializeController::is_triggered())
        {
            PreInitializeController().execute();
        }

        if (ClassInitializeController::is_triggered())
        {
            ClassInitializeController().execute();
        }

        if (ScriptEngineInitializeController::is_triggered())
        {
            ScriptEngineInitializeController().execute();
        }

        if (ConfigsPreInitializeController::is_triggered())
        {
            ConfigsPreInitializeController().execute();
        }

        if (ConfigsInitializeController::is_triggered())
        {
            ConfigsInitializeController().execute();
        }

        if (DefaultResourcesInitializeController::is_triggered())
        {
            DefaultResourcesInitializeController().execute();
        }

        if (InitializeController::is_triggered())
        {
            InitializeController().execute();
        }
    }

    LoadingControllerBase& LoadingControllerBase::push(const ControllerCallback& callback, const String& name,
                                                       const std::initializer_list<String>& require_initializers)
    {
        CallbackEntry entry;
        entry.function             = callback;
        entry.require_initializers = require_initializers;
        m_list[name].push_back(entry);
        return *this;
    }

    LoadingControllerBase& LoadingControllerBase::require(const String& name)
    {
        auto it = m_list.find(name);
        if (it == m_list.end())
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

        while (!m_list.empty())
        {
            auto id = m_list.begin();
            require(id->first);
            m_list.erase(id);
        }

        return *this;
    }


    enum class ControllerType
    {
        PreInit                = BIT(0),
        Init                   = BIT(1),
        Destroy                = BIT(2),
        PostDestroy            = BIT(3),
        ClassInit              = BIT(4),
        ResourcesInit          = BIT(5),
        ScriptEngineInitialize = BIT(6),
        ConfigsInitialize      = BIT(7),
        ConfigsPreInitialize   = BIT(8),
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


#define IMPLEMENT_CONTROLLER(ControllerName, type)                                                                               \
    ControllerName::ControllerName() : LoadingControllerBase(callbacks_list<ControllerName>(), #ControllerName)                  \
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

    IMPLEMENT_CONTROLLER(PreInitializeController, PreInit);
    IMPLEMENT_CONTROLLER(InitializeController, Init);
    //IMPLEMENT_CONTROLLER(PostInitializeController, PostInit);

    IMPLEMENT_CONTROLLER(DestroyController, Destroy);
    IMPLEMENT_CONTROLLER(PostDestroyController, PostDestroy);


    IMPLEMENT_CONTROLLER(ClassInitializeController, ClassInit);
    IMPLEMENT_CONTROLLER(DefaultResourcesInitializeController, ResourcesInit);
    IMPLEMENT_CONTROLLER(ScriptEngineInitializeController, ScriptEngineInitialize);
    IMPLEMENT_CONTROLLER(ConfigsInitializeController, ConfigsInitialize);
    IMPLEMENT_CONTROLLER(ConfigsPreInitializeController, ConfigsPreInitialize);

}// namespace Engine
