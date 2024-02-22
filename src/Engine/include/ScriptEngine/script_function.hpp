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
        asIScriptContext* m_context   = nullptr;
        asIScriptFunction* m_function = nullptr;

        ScriptFunction& bind();

    public:
        ScriptFunction(asIScriptFunction* function = nullptr);
        copy_constructors_hpp(ScriptFunction);

        bool operator==(const ScriptFunction& func) const;
        bool operator!=(const ScriptFunction& func) const;

        ScriptFunction& unbind();
        bool is_valid() const;

        ScriptFunction& prepare();
        ScriptFunction& arg_uint8(uint_t arg, uint8_t value);
        ScriptFunction& arg_uint16(uint_t arg, uint16_t value);
        ScriptFunction& arg_uint32(uint_t arg, uint32_t value);
        ScriptFunction& arg_uint64(uint_t arg, uint64_t value);
        ScriptFunction& arg_float(uint_t arg, float value);
        ScriptFunction& arg_double(uint_t arg, double value);
        ScriptFunction& arg_address(uint_t arg, void* addr);
        ScriptFunction& arg_object(uint_t arg, void* obj);
        ScriptFunction& arg_var_type(uint_t arg, void* ptr, int_t type_id);
        ScriptFunction& object(const ScriptObject& object);

        ScriptFunction& call();
        ScriptFunction& unbind_context();
        void* result_object_address();

        uint8_t result_byte();
        uint16_t result_word();
        uint32_t result_dword();
        uint64_t result_qword();
        float result_float();
        double result_double();
        void* result_address();

        int_t id() const;
        Type func_type() const;
        const char* module_name() const;
        ScriptModule module() const;
        const char* script_section_name() const;


        //        // Function signature
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


        template<typename T>
        T* result_object()
        {
            return reinterpret_cast<T*>(result_object_address());
        }

        ~ScriptFunction();

        friend class ScriptEngine;
        friend class ScriptModule;
        friend class ScriptTypeInfo;
    };

    using ScriptClassMethod = ScriptFunction;
}// namespace Engine
