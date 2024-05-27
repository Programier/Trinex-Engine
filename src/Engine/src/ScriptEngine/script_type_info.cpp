#include <Core/exception.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{

    ScriptTypeInfo::ScriptTypeInfo(asITypeInfo* info) : m_info(info)
    {}


    ScriptTypeInfo::ScriptTypeInfo(const ScriptTypeInfo& obj)
    {
        m_info = obj.m_info;
        bind();
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
            unbind();
            m_info     = obj.m_info;
            obj.m_info = nullptr;
        }

        return *this;
    }

    ScriptTypeInfo& ScriptTypeInfo::operator=(const ScriptTypeInfo& obj)
    {
        if (this != &obj)
        {
            unbind();
            m_info = obj.m_info;
            bind();
        }
        return *this;
    }

    ScriptTypeInfo& ScriptTypeInfo::bind()
    {
        if (m_info != nullptr)
        {
            m_info->AddRef();
        }
        return *this;
    }

    bool ScriptTypeInfo::is_valid() const
    {
        return m_info != nullptr;
    }

    ScriptTypeInfo& ScriptTypeInfo::unbind()
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
        return m_info->GetModule();
    }

    // Type info
    const char* ScriptTypeInfo::name() const
    {
        return m_info->GetName();
    }

    const char* ScriptTypeInfo::namespace_name() const
    {
        return m_info->GetNamespace();
    }

    ScriptTypeInfo ScriptTypeInfo::base_type() const
    {
        return ScriptTypeInfo(m_info->GetBaseType()).bind();
    }

    bool ScriptTypeInfo::derives_from(const ScriptTypeInfo& info)
    {
        return m_info->DerivesFrom(info.m_info);
    }

    int_t ScriptTypeInfo::type_id() const
    {
        return m_info->GetTypeId();
    }

    int_t ScriptTypeInfo::sub_type_id(uint_t index) const
    {
        return m_info->GetSubTypeId(index);
    }

    uint_t ScriptTypeInfo::size() const
    {
        return m_info->GetSize();
    }

    ScriptTypeInfo ScriptTypeInfo::sub_type(uint_t index) const
    {
        return ScriptTypeInfo(m_info->GetSubType(index)).bind();
    }

    uint_t ScriptTypeInfo::sub_type_count() const
    {
        return m_info->GetSubTypeCount();
    }

    // Interfaces
    uint_t ScriptTypeInfo::interface_count() const
    {
        return m_info->GetInterfaceCount();
    }

    ScriptTypeInfo ScriptTypeInfo::interface(uint_t index)
    {
        return ScriptTypeInfo(m_info->GetInterface(index)).bind();
    }

    bool ScriptTypeInfo::implements(const ScriptTypeInfo& obj_type) const
    {
        return m_info->Implements(obj_type.m_info);
    }

    // Factories
    uint_t ScriptTypeInfo::factory_count() const
    {
        return m_info->GetFactoryCount();
    }

    ScriptFunction ScriptTypeInfo::factory_by_index(uint_t index) const
    {
        return ScriptFunction(m_info->GetFactoryByIndex(index)).bind();
    }

    ScriptFunction ScriptTypeInfo::factory_by_decl(const char* decl) const
    {
        return ScriptFunction(m_info->GetFactoryByDecl(decl)).bind();
    }

    ScriptFunction ScriptTypeInfo::factory_by_decl(const String& decl) const
    {
        return factory_by_decl(decl.c_str());
    }


    // Methods
    uint_t ScriptTypeInfo::method_count() const
    {
        return m_info->GetMethodCount();
    }

    ScriptFunction ScriptTypeInfo::method_by_index(uint_t index, bool get) const
    {
        return ScriptFunction(m_info->GetMethodByIndex(index, get)).bind();
    }

    ScriptFunction ScriptTypeInfo::method_by_name(const char* name, bool get) const
    {
        return ScriptFunction(m_info->GetMethodByName(name, get)).bind();
    }

    ScriptFunction ScriptTypeInfo::method_by_decl(const char* decl, bool get) const
    {
        return ScriptFunction(m_info->GetMethodByDecl(decl, get)).bind();
    }

    ScriptFunction ScriptTypeInfo::method_by_name(const String& name, bool get) const
    {
        return method_by_name(name.c_str(), get);
    }

    ScriptFunction ScriptTypeInfo::method_by_decl(const String& decl, bool get) const
    {
        return method_by_decl(decl.c_str(), get);
    }


    // Properties
    uint_t ScriptTypeInfo::property_count() const
    {
        return m_info->GetPropertyCount();
    }

    int_t ScriptTypeInfo::property(uint_t index, String& name, int_t* type_id, bool* is_private, bool* is_protected,
                                   int_t* offset, bool* is_reference) const
    {
        const char* c_name = nullptr;
        int_t res          = m_info->GetProperty(index, &c_name, type_id, is_private, is_protected, offset, is_reference);
        if (c_name)
        {
            name = c_name;
            delete c_name;
        }
        return res;
    }

    const char* ScriptTypeInfo::property_declaration(uint_t index, bool include_bamespace) const
    {
        return m_info->GetPropertyDeclaration(index, include_bamespace);
    }

    // Behaviours
    uint_t ScriptTypeInfo::behaviour_count() const
    {
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
        asEBehaviours behaviours;
        asIScriptFunction* func = m_info->GetBehaviourByIndex(index, &behaviours);
        if (behaviour)
        {
            (*behaviour) = convert_behaviour(behaviours);
        }

        return ScriptFunction(func).bind();
    }

    // Child types
    uint_t ScriptTypeInfo::child_funcdef_count()
    {
        return m_info->GetChildFuncdefCount();
    }

    ScriptTypeInfo ScriptTypeInfo::child_funcdef(uint_t index) const
    {
        return ScriptTypeInfo(m_info->GetChildFuncdef(index)).bind();
    }

    ScriptTypeInfo ScriptTypeInfo::parent_type() const
    {
        return ScriptTypeInfo(m_info->GetParentType()).bind();
    }

    // Enums
    uint_t ScriptTypeInfo::enum_value_count() const
    {
        return m_info->GetEnumValueCount();
    }

    const char* ScriptTypeInfo::enum_value_by_index(uint_t index, int_t* out_value) const
    {
        return m_info->GetEnumValueByIndex(index, reinterpret_cast<int*>(out_value));
    }

    // Typedef
    int_t ScriptTypeInfo::typedef_type_id() const
    {
        return m_info->GetTypedefTypeId();
    }

    // Funcdef
    ScriptFunction ScriptTypeInfo::funcdef_signature() const
    {
        return ScriptFunction(m_info->GetFuncdefSignature()).bind();
    }

    bool ScriptTypeInfo::is_enum() const
    {
        return (m_info->GetFlags() & asOBJ_ENUM) == asOBJ_ENUM;
    }

    bool ScriptTypeInfo::is_array() const
    {
        return std::strcmp("array", m_info->GetName()) == 0;
    }

    ScriptTypeInfo::~ScriptTypeInfo()
    {
        unbind();
    }
}// namespace Engine
