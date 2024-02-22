#include <Core/class.hpp>
#include <Core/object.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine
{

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

    static asEBehaviours create_bahaviour(ScriptClassBehave behave)
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


    ScriptClassRegistrar::ScriptClassRegistrar(class Class* _class)
        : m_class_base_name(_class->base_name()), m_class_namespace_name(_class->namespace_name()),
          m_class_name(_class->name()), m_engine(ScriptEngine::instance()->as_engine())
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

    ScriptClassRegistrar::ScriptClassRegistrar(const String& full_name, const ClassInfo& info)
        : m_class_base_name(Object::object_name_of(full_name)), m_class_namespace_name(Object::package_name_of(full_name)),
          m_class_name(full_name), m_engine(ScriptEngine::instance()->as_engine())
    {
        m_info = info;
        declare_as_class();
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
        ScriptEngine::instance()->default_namespace(name);
    }

    static void static_declare_new_class(const String& class_namespace, const String& base_name,
                                         const ScriptClassRegistrar::ClassInfo& info)
    {
        static Set<String> registered;
        String fullname = class_namespace + "::" + base_name;
        if (!registered.contains(fullname))
        {
            asIScriptEngine* engine = ScriptEngine::instance()->as_engine();
            String ns               = engine->GetDefaultNamespace();
            assert(engine->SetDefaultNamespace(class_namespace.c_str()) >= 0);
            assert(engine->RegisterObjectType(base_name.c_str(), info.size, create_flags(info)) >= 0);
            assert(engine->SetDefaultNamespace(ns.c_str()) >= 0);
            registered.insert(fullname);
        }
    }


    void ScriptClassRegistrar::declare_as_class(Class* _class)
    {
        ClassInfo info;
        info      = {};
        info.size = _class->sizeof_class();
        declare_as_class(_class, info);
    }

    void ScriptClassRegistrar::declare_as_class(Class* _class, const ClassInfo& info)
    {
        static_declare_new_class(_class->namespace_name(), _class->base_name(), info);

        // Register cast operators

        asIScriptEngine* engine = ScriptEngine::instance()->as_engine();
        for (Class* parent = _class->parent(); parent != nullptr; parent = parent->parent())
        {
            if (parent->is_binded_to_script())
            {
                String op = Strings::format("{}@ opCast()", _class->name().c_str());
                assert(engine->RegisterObjectMethod(parent->name().c_str(), op.c_str(), asFUNCTION(parent->cast_to_this()),
                                                    asCALL_CDECL_OBJLAST) >= 0);

                op = Strings::format("{}@ opImplCast()", parent->name().c_str());
                assert(engine->RegisterObjectMethod(_class->name().c_str(), op.c_str(), asFUNCTION(_class->cast_to_this()),
                                                    asCALL_CDECL_OBJLAST) >= 0);
            }
        }
    }

    ScriptClassRegistrar& ScriptClassRegistrar::private_register_method(const char* declaration, void* method,
                                                                        ScriptCallConv conv)
    {

        prepare_namespace();
        assert(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration, create_function(method, FuncType::Method),
                                               create_call_conv(conv)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::private_register_virtual_method(const char* declaration, void* method,
                                                                                ScriptCallConv conv)
    {
        prepare_namespace();
        assert(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration, create_function(method, FuncType::Func),
                                               create_call_conv(conv)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::private_register_static_method(const char* declaration, void* func,
                                                                               ScriptCallConv conv)
    {
        prepare_namespace(true);
        assert(m_engine->RegisterGlobalFunction(declaration, create_function(func, FuncType::Func), create_call_conv(conv)) >=
               0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::property(const char* declaration, void* prop)
    {
        prepare_namespace();
        assert(m_engine->RegisterObjectProperty(m_class_base_name.c_str(), declaration, reinterpret_cast<size_t>(prop)) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::static_property(const char* declaration, void* prop)
    {
        prepare_namespace(true);
        assert(m_engine->RegisterGlobalProperty(declaration, prop) >= 0);
        return release_namespace();
    }

    ScriptClassRegistrar& ScriptClassRegistrar::require_type(const String& name, const ClassInfo& info)
    {
        String ns = Object::package_name_of(name), bs = Object::object_name_of(name);
        static_declare_new_class(ns, bs, info);
        return *this;
    }

    ScriptClassRegistrar& ScriptClassRegistrar::private_register_behaviour(ScriptClassBehave behave, const char* declaration,
                                                                           void* method, bool is_method, ScriptCallConv conv)
    {
        prepare_namespace();
        asSFuncPtr ptr = create_function(method, is_method ? FuncType::Method : FuncType::Func);
        assert(m_engine->RegisterObjectBehaviour(m_class_base_name.c_str(), create_bahaviour(behave), declaration, ptr,
                                                  create_call_conv(conv)) >= 0);
        return release_namespace();
    }


    ScriptClassRegistrar& ScriptClassRegistrar::prepare_namespace(bool static_member)
    {
        m_current_namespace = m_engine->GetDefaultNamespace();
        if (static_member)
        {
            assert(m_engine->SetDefaultNamespace(m_class_name.c_str()) >= 0);
        }
        else
        {
            assert(m_engine->SetDefaultNamespace(m_class_namespace_name.c_str()) >= 0);
        }
        return *this;
    }

    ScriptClassRegistrar& ScriptClassRegistrar::release_namespace()
    {
        assert(m_engine->SetDefaultNamespace(m_current_namespace.c_str()) >= 0);
        return *this;
    }

    ScriptClassRegistrar& ScriptClassRegistrar::declare_as_class()
    {
        static_declare_new_class(m_class_namespace_name, m_class_base_name, m_info);
        return *this;
    }


    ScriptEnumRegistrar::ScriptEnumRegistrar(const String& full_name)
        : m_enum_base_name(Object::object_name_of(full_name)), m_enum_namespace_name(Object::package_name_of(full_name)),
          m_engine(ScriptEngine::instance()->as_engine())
    {
        prepare_namespace();
        assert(m_engine->RegisterEnum(m_enum_base_name.c_str()) >= 0);
        release_namespace();
    }


    ScriptEnumRegistrar& ScriptEnumRegistrar::prepare_namespace()
    {
        m_current_namespace = m_engine->GetDefaultNamespace();
        assert(m_engine->SetDefaultNamespace(m_enum_namespace_name.c_str()) >= 0);
        return *this;
    }

    ScriptEnumRegistrar& ScriptEnumRegistrar::release_namespace()
    {
        assert(m_engine->SetDefaultNamespace(m_current_namespace.c_str()) >= 0);
        return *this;
    }

    ScriptEnumRegistrar& ScriptEnumRegistrar::set(const char* name, int_t value)
    {
        prepare_namespace();
        assert(m_engine->RegisterEnumValue(m_enum_base_name.c_str(), name, value) >= 0);
        return release_namespace();
    }


    ScriptClassRegistrar& ScriptClassRegistrar::private_register_operator(const char* declaration, void* method, bool is_method,
                                                                          ScriptCallConv conv)
    {
        asSFuncPtr ptr = create_function(method, is_method ? Method : Func);
        prepare_namespace();
        assert(m_engine->RegisterObjectMethod(m_class_base_name.c_str(), declaration, ptr, create_call_conv(conv)) >= 0);
        return release_namespace();
    }

}// namespace Engine
