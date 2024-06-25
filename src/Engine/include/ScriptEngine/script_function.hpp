#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

class asIScriptContext;
class asIScriptFunction;

namespace Engine
{
    class ScriptTypeInfo;
    class ScriptModule;
    class ScriptEngine;
    class ScriptObject;

    class ENGINE_EXPORT ScriptFunction
    {
    public:
        enum Type
        {
            Dummy     = -1,
            System    = 0,
            Script    = 1,
            Interface = 2,
            Virtual   = 3,
            Funcdef   = 4,
            Imported  = 5,
            Delegate  = 6
        };

    private:
        mutable asIScriptFunction* m_function = nullptr;
        const ScriptFunction& add_ref() const;

    public:
        ScriptFunction(asIScriptFunction* function = nullptr);
        copy_constructors_hpp(ScriptFunction);


        asIScriptFunction* function() const;
        bool operator==(const ScriptFunction& func) const;
        bool operator!=(const ScriptFunction& func) const;

        bool is_valid() const;
        const ScriptFunction& release() const;

        int_t id() const;
        Type func_type() const;
        const char* module_name() const;
        ScriptModule module() const;
        const char* script_section_name() const;

        // Function signature
        ScriptTypeInfo object_type() const;
        const char* object_name() const;
        const char* name() const;
        const char* namespace_name() const;
        const char* declaration(bool include_object_name = true, bool include_namespace = false,
                                bool include_param_names = false) const;
        bool is_read_only() const;
        bool is_private() const;
        bool is_protected() const;
        bool is_final() const;
        bool is_override() const;
        bool is_shared() const;
        bool is_explicit() const;
        bool is_property() const;
        uint_t param_count() const;

        // Type id for function pointers
        int_t type_id() const;
        bool is_compatible_with_type_id(int_t type_id) const;

        // Delegates
        void* delegate_object() const;
        ScriptTypeInfo delegate_object_type() const;
        ScriptFunction delegate_function() const;
        ~ScriptFunction();
    };

    using ScriptClassMethod = ScriptFunction;
}// namespace Engine
