#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine
{
#define SCRIPT_CALL(call) trinex_always_check(call, #call)

    enum FuncType
    {
        General = 1,
        Func    = 2,
        Method  = 3,
    };


    struct Dummy {
    };

    asSFuncPtr create_function(void* data, FuncType type)
    {
        if (type == Func)
        {
            return asFunctionPtr(data);
        }

        if (type == Method)
        {
            using Helper = void (Dummy::*)();

            Helper* helper = reinterpret_cast<Helper*>(data);
            return asSMethodPtr<sizeof(void(Dummy::*)())>::Convert((void(Dummy::*)())(*helper));
        }

        return {};
    }


    static asDWORD create_flags(const ScriptClassRegistrar::ClassInfo& info)
    {
        asDWORD flags = static_cast<asDWORD>(info.flags);
        // If class info haven't any flags, using Ref flag for registration
        if (!flags)
        {
            return ScriptClassRegistrar::Ref;
        }
        return flags;
    }

    static asDWORD create_call_conv(ScriptCallConv conv)
    {
        switch (conv)
        {
            case ScriptCallConv::CDECL:
                return asCALL_CDECL;
            case ScriptCallConv::STDCALL:
                return asCALL_STDCALL;
            case ScriptCallConv::THISCALL_ASGLOBAL:
                return asCALL_THISCALL_ASGLOBAL;
            case ScriptCallConv::THISCALL:
                return asCALL_THISCALL;
            case ScriptCallConv::CDECL_OBJLAST:
                return asCALL_CDECL_OBJLAST;
            case ScriptCallConv::CDECL_OBJFIRST:
                return asCALL_CDECL_OBJFIRST;
            case ScriptCallConv::GENERIC:
                return asCALL_GENERIC;
            case ScriptCallConv::THISCALL_OBJLAST:
                return asCALL_THISCALL_OBJLAST;
            case ScriptCallConv::THISCALL_OBJFIRST:
                return asCALL_THISCALL_OBJFIRST;
            default:
                throw EngineException("Undefined call convension!");
        }
    }

    static asEBehaviours create_behaviour(ScriptClassBehave behave)
    {
        switch (behave)
        {
            case ScriptClassBehave::Construct:
                return asBEHAVE_CONSTRUCT;
            case ScriptClassBehave::ListConstruct:
                return asBEHAVE_LIST_CONSTRUCT;
            case ScriptClassBehave::Destruct:
                return asBEHAVE_DESTRUCT;
            case ScriptClassBehave::Factory:
                return asBEHAVE_FACTORY;
            case ScriptClassBehave::ListFactory:
                return asBEHAVE_LIST_FACTORY;
            case ScriptClassBehave::AddRef:
                return asBEHAVE_ADDREF;
            case ScriptClassBehave::Release:
                return asBEHAVE_RELEASE;
            case ScriptClassBehave::GetWeakRefFlag:
                return asBEHAVE_GET_WEAKREF_FLAG;
            case ScriptClassBehave::TemplateCallback:
                return asBEHAVE_TEMPLATE_CALLBACK;
            case ScriptClassBehave::GetRefCount:
                return asBEHAVE_GETREFCOUNT;
            case ScriptClassBehave::GetGCFlag:
                return asBEHAVE_GETGCFLAG;
            case ScriptClassBehave::SetGCFlag:
                return asBEHAVE_SETGCFLAG;
            case ScriptClassBehave::EnumRefs:
                return asBEHAVE_ENUMREFS;
            case ScriptClassBehave::ReleaseRefs:
                return asBEHAVE_RELEASEREFS;
            default:
                throw EngineException("Undefined behave!");
        }
    }


    ScriptClassRegistrar::ScriptClassRegistrar(const class Class* _class)
        : m_class_base_name(_class->base_name()), m_class_namespace_name(_class->namespace_name()), m_class_name(_class->name()),
          m_engine(ScriptEngine::engine())
    {
        m_info      = {};
        m_info.size = _class->sizeof_class();
        declare_as_class(_class, m_info);
    }


    template<class B>
    static B* ref_cast(Object* a)
    {
        return a->instance_cast<B>();
    }

    ScriptClassRegistrar::ScriptClassRegistrar(const String& full_name, const ClassInfo& info, const String& template_override)
        : m_class_base_name(Object::object_name_of(full_name)), m_class_namespace_name(Object::package_name_of(full_name)),
          m_class_name(full_name), m_engine(ScriptEngine::engine())
    {
        m_info = info;
        declare_as_class();
        if (!template_override.empty() && (info.flags & Template) == Template)
        {
            m_class_base_name = template_override;
            m_class_name = m_class_namespace_name.empty() ? m_class_base_name : m_class_namespace_name + "::" + m_class_base_name;
        }
    }

    const String& ScriptClassRegistrar::namespace_name() const
    {
        return m_class_namespace_name;
    }

    const String& ScriptClassRegistrar::class_base_name() const
    {
        return m_class_base_name;
    }

    const String& ScriptClassRegistrar::class_name() const
    {
        return m_class_name;
    }


    ENGINE_EXPORT void ScriptClassRegistrar::global_namespace_name(const String& name)
    {
        ScriptEngine::default_namespace(name);
    }

    static void static_declare_new_class(const String& class_namespace, const String& base_name,
                                         const ScriptClassRegistrar::ClassInfo& info)
    {
        static Set<String> registered;
        String fullname = class_namespace.empty() ? base_name : class_namespace + "::" + base_name;

        if (!registered.contains(fullname))
        {
            asIScriptEngine* engine = ScriptEngine::engine();
            String ns               = engine->GetDefaultNamespace();

            SCRIPT_CALL(engine->SetDefaultNamespace(class_namespace.c_str()) >= 0);
            SCRIPT_CALL(engine->RegisterObjectType(base_name.c_str(), info.size, create_flags(info)) >= 0);
            SCRIPT_CALL(engine->SetDefaultNamespace(ns.c_str()) >= 0);
            registered.insert(fullname);
        }
    }


    void ScriptClassRegistrar::declare_as_class(const Class* _class)
    {
        ClassInfo info;
        info      = {};
        info.size = _class->sizeof_class();
        declare_as_class(_class, info);
    }

    void ScriptClassRegistrar::declare_as_class(const Class* _class, const ClassInfo& info)
    {
        static_declare_new_class(_class->namespace_name(), _class->base_name(), info);

        // Register cast operators

        asIScriptEngine* engine = ScriptEngine::engine();
        for (Class* parent = _class->parent(); parent != nullptr; parent = parent->parent())
        {
            if (parent->is_scriptable())
            {
                assert(1 > 0);
                String op = Strings::format("{}@ opCast()", _class->name().c_str());
                SCRIPT_CALL(engine->RegisterObjectMethod(parent->name().c_str(), op.c_str(), asFUNCTION(parent->cast_to_this()),
                                                         asCALL_CDECL_OBJLAST) >= 0);

                op = Strings::format("{}@ opImplCast()", parent->name().c_str());
                SCRIPT_CALL(engine->RegisterObjectMethod(_class->name().c_str(), op.c_str(), asFUNCTION(_class->cast_to_this()),
                                                         asCALL_CDECL_OBJLAST) >= 0);
            }
        }
    }

    ScriptClassRegistrar& ScriptClassRegistrar::method(const char* declaration, ScriptMethodPtr* method, ScriptCallConv conv)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration, *reinterpret_cast<asSFuncPtr*>(method),
                                                   create_call_conv(conv)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::method(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration,
                                                   *reinterpret_cast<asSFuncPtr*>(function), create_call_conv(conv)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::static_function(const char* declaration, ScriptFuncPtr* function,
                                                                ScriptCallConv conv)
    {
        prepare_namespace(true);
        ScriptEngine::register_function(declaration, function, conv);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::property(const char* declaration, size_t offset)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectProperty(m_class_base_name.c_str(), declaration, offset) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::static_property(const char* declaration, void* prop)
    {
        prepare_namespace(true);
        SCRIPT_CALL(m_engine->RegisterGlobalProperty(declaration, prop) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::require_type(const String& name, const ClassInfo& info)
    {
        String ns = Object::package_name_of(name), bs = Object::object_name_of(name);
        static_declare_new_class(ns, bs, info);
        return *this;
    }

    ScriptClassRegistrar& ScriptClassRegistrar::behave(ScriptClassBehave behaviour, const char* declaration,
                                                       ScriptFuncPtr* function, ScriptCallConv conv)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectBehaviour(m_class_base_name.c_str(), create_behaviour(behaviour), declaration,
                                                      *reinterpret_cast<asSFuncPtr*>(function), create_call_conv(conv)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::behave(ScriptClassBehave behaviour, const char* declaration,
                                                       ScriptMethodPtr* method, ScriptCallConv conv)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectBehaviour(m_class_base_name.c_str(), create_behaviour(behaviour), declaration,
                                                      *reinterpret_cast<asSFuncPtr*>(method), create_call_conv(conv)) >= 0);
        return release_namespace();
    }


    ScriptClassRegistrar& ScriptClassRegistrar::prepare_namespace(bool static_member)
    {
        m_current_namespace = m_engine->GetDefaultNamespace();
        if (static_member)
        {
            SCRIPT_CALL(m_engine->SetDefaultNamespace(m_class_name.c_str()) >= 0);
        }
        else
        {
            SCRIPT_CALL(m_engine->SetDefaultNamespace(m_class_namespace_name.c_str()) >= 0);
        }
        return *this;
    }

    ScriptClassRegistrar& ScriptClassRegistrar::release_namespace()
    {
        SCRIPT_CALL(m_engine->SetDefaultNamespace(m_current_namespace.c_str()) >= 0);
        return *this;
    }

    ScriptClassRegistrar& ScriptClassRegistrar::declare_as_class()
    {
        static_declare_new_class(m_class_namespace_name, m_class_base_name, m_info);
        return *this;
    }


    ScriptEnumRegistrar::ScriptEnumRegistrar(const String& namespace_name, const String& base_name, bool init)
        : m_enum_base_name(base_name), m_enum_namespace_name(namespace_name), m_engine(ScriptEngine::engine())
    {
        if (init)
        {
            prepare_namespace();
            SCRIPT_CALL(m_engine->RegisterEnum(m_enum_base_name.c_str()) >= 0);
            release_namespace();
        }
    }

    ScriptEnumRegistrar::ScriptEnumRegistrar(const String& full_name, bool init)
        : Engine::ScriptEnumRegistrar(Strings::namespace_of(full_name), Strings::class_name_of(full_name), init)
    {}


    ScriptEnumRegistrar& ScriptEnumRegistrar::prepare_namespace()
    {
        m_current_namespace = m_engine->GetDefaultNamespace();
        SCRIPT_CALL(m_engine->SetDefaultNamespace(m_enum_namespace_name.c_str()) >= 0);
        return *this;
    }

    ScriptEnumRegistrar& ScriptEnumRegistrar::release_namespace()
    {
        SCRIPT_CALL(m_engine->SetDefaultNamespace(m_current_namespace.c_str()) >= 0);
        return *this;
    }

    ScriptEnumRegistrar& ScriptEnumRegistrar::set(const char* name, int_t value)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterEnumValue(m_enum_base_name.c_str(), name, value) >= 0);
        return release_namespace();
    }


    ScriptClassRegistrar& ScriptClassRegistrar::opfunc(const char* declaration, ScriptMethodPtr* method, ScriptCallConv conv)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration, *reinterpret_cast<asSFuncPtr*>(method),
                                                   create_call_conv(conv)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::opfunc(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv)
    {
        prepare_namespace();
        SCRIPT_CALL(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration,
                                                   *reinterpret_cast<asSFuncPtr*>(function), create_call_conv(conv)) >= 0);
        return release_namespace();
    }
}// namespace Engine
