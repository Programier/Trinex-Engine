#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_handle.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_primitives.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{
#define check_info(return_value)                                                                                                 \
    if (m_info == nullptr)                                                                                                       \
    return return_value


    ScriptTypeInfo::ScriptTypeInfo(asITypeInfo* info) : m_info(info)
    {
        add_ref();
    }


    ScriptTypeInfo::ScriptTypeInfo(const ScriptTypeInfo& obj)
    {
        m_info = obj.m_info;
        add_ref();
    }

    ScriptTypeInfo::ScriptTypeInfo(ScriptTypeInfo&& obj)
    {
        m_info     = obj.m_info;
        obj.m_info = nullptr;
    }

    ScriptTypeInfo& ScriptTypeInfo::operator=(ScriptTypeInfo&& obj)
    {
        if (this != &obj)
        {
            release();
            m_info     = obj.m_info;
            obj.m_info = nullptr;
        }

        return *this;
    }

    ScriptTypeInfo& ScriptTypeInfo::operator=(const ScriptTypeInfo& obj)
    {
        if (this != &obj)
        {
            release();
            m_info = obj.m_info;
            add_ref();
        }
        return *this;
    }

    const ScriptTypeInfo& ScriptTypeInfo::add_ref() const
    {
        if (m_info != nullptr)
        {
            m_info->AddRef();
        }
        return *this;
    }

    asITypeInfo* ScriptTypeInfo::info() const
    {
        return m_info;
    }

    bool ScriptTypeInfo::is_valid() const
    {
        return m_info != nullptr;
    }

    const ScriptTypeInfo& ScriptTypeInfo::release() const
    {
        if (m_info)
        {
            m_info->Release();
            m_info = nullptr;
        }

        return *this;
    }

    // Miscellaneous
    ScriptModule ScriptTypeInfo::module() const
    {
        check_info({});
        return m_info->GetModule();
    }

    // Type info
    StringView ScriptTypeInfo::name() const
    {
        check_info("");
        return Strings::make_string_view(m_info->GetName());
    }

    StringView ScriptTypeInfo::namespace_name() const
    {
        check_info("");
        return Strings::make_string_view(m_info->GetNamespace());
    }

    ScriptTypeInfo ScriptTypeInfo::base_type() const
    {
        check_info({});
        return ScriptTypeInfo(m_info->GetBaseType());
    }

    bool ScriptTypeInfo::derives_from(const ScriptTypeInfo& info) const
    {
        check_info(false);
        return m_info->DerivesFrom(info.m_info);
    }

    int_t ScriptTypeInfo::type_id() const
    {
        check_info(0);
        return m_info->GetTypeId();
    }

    int_t ScriptTypeInfo::sub_type_id(uint_t index) const
    {
        check_info(0);
        return m_info->GetSubTypeId(index);
    }

    uint_t ScriptTypeInfo::size() const
    {
        check_info(0);
        return m_info->GetSize();
    }

    ScriptTypeInfo ScriptTypeInfo::sub_type(uint_t index) const
    {
        check_info({});
        return ScriptTypeInfo(m_info->GetSubType(index));
    }

    uint_t ScriptTypeInfo::sub_type_count() const
    {
        check_info(0);
        return m_info->GetSubTypeCount();
    }

    // Interfaces
    uint_t ScriptTypeInfo::interface_count() const
    {
        check_info(0);
        return m_info->GetInterfaceCount();
    }

    ScriptTypeInfo ScriptTypeInfo::interface(uint_t index)
    {
        check_info({});
        return ScriptTypeInfo(m_info->GetInterface(index));
    }

    bool ScriptTypeInfo::implements(const ScriptTypeInfo& obj_type) const
    {
        check_info(false);
        return m_info->Implements(obj_type.m_info);
    }

    // Factories
    uint_t ScriptTypeInfo::factory_count() const
    {
        check_info(0);
        return m_info->GetFactoryCount();
    }

    ScriptFunction ScriptTypeInfo::factory_by_index(uint_t index) const
    {
        check_info({});
        return ScriptFunction(m_info->GetFactoryByIndex(index));
    }

    ScriptFunction ScriptTypeInfo::factory_by_decl(const char* decl) const
    {
        check_info({});
        return ScriptFunction(m_info->GetFactoryByDecl(decl));
    }

    ScriptFunction ScriptTypeInfo::factory_by_decl(const String& decl) const
    {
        check_info({});
        return factory_by_decl(decl.c_str());
    }

    // Methods
    uint_t ScriptTypeInfo::method_count() const
    {
        check_info({});
        return m_info->GetMethodCount();
    }

    ScriptFunction ScriptTypeInfo::method_by_index(uint_t index, bool get) const
    {
        check_info({});
        return ScriptFunction(m_info->GetMethodByIndex(index, get));
    }

    ScriptFunction ScriptTypeInfo::method_by_name(const char* name, bool get) const
    {
        check_info({});
        return ScriptFunction(m_info->GetMethodByName(name, get));
    }

    ScriptFunction ScriptTypeInfo::method_by_decl(const char* decl, bool get) const
    {
        check_info({});
        return ScriptFunction(m_info->GetMethodByDecl(decl, get));
    }

    ScriptFunction ScriptTypeInfo::method_by_name(const String& name, bool get) const
    {
        check_info({});
        return method_by_name(name.c_str(), get);
    }

    ScriptFunction ScriptTypeInfo::method_by_decl(const String& decl, bool get) const
    {
        check_info({});
        return method_by_decl(decl.c_str(), get);
    }


    // Properties
    uint_t ScriptTypeInfo::property_count() const
    {
        check_info(0);
        return m_info->GetPropertyCount();
    }

    bool ScriptTypeInfo::property(uint_t index, StringView* name, int_t* type_id, bool* is_private, bool* is_protected,
                                  int_t* offset, bool* is_reference) const
    {
        check_info(false);
        const char* c_name = nullptr;
        int_t res = m_info->GetProperty(index, name ? &c_name : nullptr, type_id, is_private, is_protected, offset, is_reference);

        if (name)
        {
            (*name) = Strings::make_string_view(c_name);
        }

        return res >= 0;
    }

    String ScriptTypeInfo::property_declaration(uint_t index, bool include_bamespace) const
    {
        check_info("");
        return Strings::make_string(m_info->GetPropertyDeclaration(index, include_bamespace));
    }

    // Behaviours
    uint_t ScriptTypeInfo::behaviour_count() const
    {
        check_info(0);
        return m_info->GetBehaviourCount();
    }

    static ScriptClassBehave convert_behaviour(asEBehaviours b)
    {
        switch (b)
        {
            case asBEHAVE_CONSTRUCT:
                return ScriptClassBehave::Construct;
            case asBEHAVE_LIST_CONSTRUCT:
                return ScriptClassBehave::ListConstruct;
            case asBEHAVE_DESTRUCT:
                return ScriptClassBehave::Destruct;
            case asBEHAVE_FACTORY:
                return ScriptClassBehave::Factory;
            case asBEHAVE_LIST_FACTORY:
                return ScriptClassBehave::ListFactory;
            case asBEHAVE_ADDREF:
                return ScriptClassBehave::AddRef;
            case asBEHAVE_RELEASE:
                return ScriptClassBehave::Release;
            case asBEHAVE_GET_WEAKREF_FLAG:
                return ScriptClassBehave::GetWeakRefFlag;
            case asBEHAVE_TEMPLATE_CALLBACK:
                return ScriptClassBehave::TemplateCallback;
            case asBEHAVE_GETREFCOUNT:
                return ScriptClassBehave::GetRefCount;
            case asBEHAVE_SETGCFLAG:
                return ScriptClassBehave::SetGCFlag;
            case asBEHAVE_GETGCFLAG:
                return ScriptClassBehave::GetGCFlag;
            case asBEHAVE_ENUMREFS:
                return ScriptClassBehave::EnumRefs;
            case asBEHAVE_RELEASEREFS:
                return ScriptClassBehave::ReleaseRefs;
            default:
                throw EngineException("Undefined behave");
        }

        return {};
    }

    ScriptFunction ScriptTypeInfo::behaviour_by_index(uint_t index, ScriptClassBehave* behaviour) const
    {
        check_info({});
        asEBehaviours behaviours;
        asIScriptFunction* func = m_info->GetBehaviourByIndex(index, &behaviours);
        if (behaviour)
        {
            (*behaviour) = convert_behaviour(behaviours);
        }

        return ScriptFunction(func);
    }

    // Child types
    uint_t ScriptTypeInfo::child_funcdef_count()
    {
        check_info(0);
        return m_info->GetChildFuncdefCount();
    }

    ScriptTypeInfo ScriptTypeInfo::child_funcdef(uint_t index) const
    {
        check_info({});
        return ScriptTypeInfo(m_info->GetChildFuncdef(index));
    }

    ScriptTypeInfo ScriptTypeInfo::parent_type() const
    {
        check_info({});
        return ScriptTypeInfo(m_info->GetParentType());
    }

    // Enums
    uint_t ScriptTypeInfo::enum_value_count() const
    {
        check_info(0);
        return m_info->GetEnumValueCount();
    }

    StringView ScriptTypeInfo::enum_value_by_index(uint_t index, int_t* out_value) const
    {
        check_info("");
        return Strings::make_string_view(m_info->GetEnumValueByIndex(index, reinterpret_cast<int*>(out_value)));
    }

    // Typedef
    int_t ScriptTypeInfo::typedef_type_id() const
    {
        check_info(0);
        return m_info->GetTypedefTypeId();
    }

    // Funcdef
    ScriptFunction ScriptTypeInfo::funcdef_signature() const
    {
        check_info({});
        return ScriptFunction(m_info->GetFuncdefSignature());
    }

    bool ScriptTypeInfo::is_script_object() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_SCRIPT_OBJECT) == asOBJ_SCRIPT_OBJECT;
    }

    bool ScriptTypeInfo::is_shared() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_SHARED) == asOBJ_SHARED;
    }

    bool ScriptTypeInfo::is_noinherit() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_NOINHERIT) == asOBJ_NOINHERIT;
    }

    bool ScriptTypeInfo::is_funcdef() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_FUNCDEF) == asOBJ_FUNCDEF;
    }

    bool ScriptTypeInfo::is_template_subtype() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_TEMPLATE_SUBTYPE) == asOBJ_TEMPLATE_SUBTYPE;
    }

    bool ScriptTypeInfo::is_typedef() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_TYPEDEF) == asOBJ_TYPEDEF;
    }

    bool ScriptTypeInfo::is_abstract() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_ABSTRACT) == asOBJ_ABSTRACT;
    }

    bool ScriptTypeInfo::is_enum() const
    {
        check_info(false);
        return (m_info->GetFlags() & asOBJ_ENUM) == asOBJ_ENUM;
    }

    bool ScriptTypeInfo::is_array() const
    {
        check_info(false);
        return std::strcmp("array", m_info->GetName()) == 0;
    }

    bool ScriptTypeInfo::is_object(bool handle_is_object) const
    {
        return ScriptEngine::is_object_type(type_id());
    }

    bool ScriptTypeInfo::is_handle() const
    {
        return ScriptEngine::is_handle_type(type_id());
    }


    ScriptTypeInfo::operator bool() const
    {
        return is_valid();
    }

    ScriptTypeInfo::~ScriptTypeInfo()
    {
        release();
    }

    template<typename T>
    static decltype(T::value)* address_of_script_var(T* value)
    {
        if (value)
        {
            return &value->value;
        }
        return nullptr;
    }


    static bool script_property(const ScriptTypeInfo* self, uint_t index, ScriptPointer* name, Integer32* type_id = nullptr,
                                Boolean* is_private = nullptr, Boolean* is_protected = nullptr, Integer32* offset = nullptr,
                                Boolean* is_reference = nullptr)
    {
        StringView* out_name = name ? name->as<StringView>() : nullptr;
        return self->property(index, out_name, address_of_script_var(type_id), address_of_script_var(is_private),
                              address_of_script_var(is_protected), address_of_script_var(offset),
                              address_of_script_var(is_reference));
    }

    static StringView script_enum_value_by_index(const ScriptTypeInfo* self, uint_t index, Integer32* out_value)
    {
        return self->enum_value_by_index(index, address_of_script_var(out_value));
    }

    static ScriptFunction script_behaviour_by_index(const ScriptTypeInfo* self, uint_t index, ScriptPointer* pointer)
    {
        ScriptClassBehave* behave = pointer ? pointer->as<ScriptClassBehave>() : nullptr;
        return self->behaviour_by_index(index, behave);
    }

    static void on_init()
    {
        ScriptClassRegistrar::ClassInfo info = ScriptClassRegistrar::create_type_info<ScriptTypeInfo>(
                ScriptClassRegistrar::Value | ScriptClassRegistrar::AppClassAllInts);

        ScriptClassRegistrar registrar("Engine::ScriptTypeInfo", info);
        ReflectionInitializeController().require("Engine::ScriptFunction").require("Engine::ScriptModule");

        using T = ScriptTypeInfo;

        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<T>);
        registrar.behave(ScriptClassBehave::Construct, "void f(const ScriptTypeInfo)",
                         ScriptClassRegistrar::constructor<T, const T&>);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<T>);
        registrar.opfunc("ScriptTypeInfo& opAssign(const ScriptTypeInfo& in)",
                         method_of<ScriptTypeInfo&, const T&>(&T::operator=));

        registrar.method("bool is_valid() const", &T::is_valid);
        registrar.method("ScriptModule module() const", &T::module);
        registrar.method("StringView name() const", &T::name);
        registrar.method("StringView namespace_name() const", &T::namespace_name);
        registrar.method("ScriptTypeInfo base_type() const", &T::base_type);
        registrar.method("bool derives_from(const ScriptTypeInfo& in) const", &T::derives_from);
        registrar.method("int type_id() const", &T::type_id);
        registrar.method("int sub_type_id(uint index) const", &T::sub_type);
        registrar.method("int size() const", &T::size);
        registrar.method("ScriptTypeInfo sub_type(uint index) const", &T::sub_type);
        registrar.method("uint sub_type_count() const", &T::sub_type_count);
        registrar.method("uint interface_count() const", &T::interface_count);
        registrar.method("ScriptTypeInfo interface_by_index(uint index) const", &T::interface);
        registrar.method("bool implements(const ScriptTypeInfo& in) const", &T::implements);
        registrar.method("uint factory_count() const", &T::factory_count);
        registrar.method("ScriptFunction factory_by_index(uint index) const", &T::factory_by_index);
        registrar.method("ScriptFunction factory_by_decl(const string& decl) const",
                         method_of<ScriptFunction, const String&>(&T::factory_by_decl));
        registrar.method("uint behaviour_count() const", &T::behaviour_count);
        registrar.method("ScriptFunction behaviour_by_index(uint index, Ptr<Engine::ScriptClassBehave>@ behave = null) const",
                         script_behaviour_by_index);
        registrar.method("uint method_count() const", &T::method_count);
        registrar.method("ScriptFunction method_by_index(uint index, bool get_virtual = true) const", &T::method_by_index);
        registrar.method("ScriptFunction method_by_name(const string& name, bool get_virtual = true) const",
                         method_of<ScriptFunction, const String&, bool>(&T::method_by_name));
        registrar.method("ScriptFunction method_by_decl(const string& decl, bool get_virtual = true) const",
                         method_of<ScriptFunction, const String&, bool>(&T::method_by_decl));
        registrar.method("uint property_count() const", &T::property_count);
        registrar.method("bool property(uint index, Ptr<StringView>@ name = null, Int32@ type_id = null, Bool@ is_private = "
                         "null, Bool@ is_protected = null, Int32@ offset = null, Bool@ is_reference = null)",
                         script_property);
        registrar.method("string property_declaration(uint index, bool include_namespace = false) const",
                         &T::property_declaration);
        registrar.method("uint child_funcdef_count()", &T::child_funcdef_count);
        registrar.method("ScriptTypeInfo child_funcdef(uint index) const", &T::child_funcdef);
        registrar.method("ScriptTypeInfo parent_type() const", &T::parent_type);
        registrar.method("uint enum_value_count() const", &T::enum_value_count);
        registrar.method("StringView enum_value_by_index() const", script_enum_value_by_index);
        registrar.method("int typedef_type_id() const", &T::typedef_type_id);
        registrar.method("ScriptFunction funcdef_signature() const", &T::funcdef_signature);
        registrar.method("bool is_script_object() const", &T::is_script_object);
        registrar.method("bool is_shared() const", &T::is_shared);
        registrar.method("bool is_noinherit() const", &T::is_noinherit);
        registrar.method("bool is_funcdef() const", &T::is_funcdef);
        registrar.method("bool is_template_subtype() const", &T::is_template_subtype);
        registrar.method("bool is_typedef() const", &T::is_typedef);
        registrar.method("bool is_abstract() const", &T::is_abstract);
        registrar.method("bool is_enum() const", &T::is_enum);
        registrar.method("bool is_array() const", &T::is_array);
    }

    static ReflectionInitializeController initializer(on_init, "Engine::ScriptTypeInfo", {"Engine::ScriptEnums"});
}// namespace Engine
