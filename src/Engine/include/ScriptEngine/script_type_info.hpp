#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <ScriptEngine/script_enums.hpp>

class asITypeInfo;

namespace Engine
{
    class ScriptModule;
    class ScriptFunction;

    class ENGINE_EXPORT ScriptTypeInfo
    {
    private:
        asITypeInfo* m_info = nullptr;

        ScriptTypeInfo& bind();

    public:
        ScriptTypeInfo(asITypeInfo* info = nullptr);
        copy_constructors_hpp(ScriptTypeInfo);
        bool is_valid() const;
        ScriptTypeInfo& unbind();
        // Miscellaneous
        ScriptModule module() const;

        // Type info
        const char* name() const;
        const char* namespace_name() const;
        ScriptTypeInfo base_type() const;
        bool derives_from(const ScriptTypeInfo& info);
        int_t type_id() const;
        int_t sub_type_id(uint_t index) const;
        uint_t size() const;
        ScriptTypeInfo sub_type(uint_t index) const;
        uint_t sub_type_count() const;


        // Interfaces
        uint_t interface_count() const;
        ScriptTypeInfo interface(uint_t index);
        bool implements(const ScriptTypeInfo& obj_type) const;

        // Factories
        uint_t factory_count() const;
        ScriptFunction factory_by_index(uint_t index) const;
        ScriptFunction factory_by_decl(const char* decl) const;
        ScriptFunction factory_by_decl(const String& decl) const;

        // Methods
        uint_t method_count() const;
        ScriptFunction method_by_index(uint_t index, bool get_virtual = true) const;
        ScriptFunction method_by_name(const char* name, bool get_virtual = true) const;
        ScriptFunction method_by_decl(const char* decl, bool get_virtual = true) const;
        ScriptFunction method_by_name(const String& name, bool get_virtual = true) const;
        ScriptFunction method_by_decl(const String& decl, bool get_virtual = true) const;

        // Properties
        uint_t property_count() const;
        int_t property(uint_t index, String& name, int_t* type_id = 0, bool* is_private = 0, bool* is_protected = 0,
                       int_t* offset = 0, bool* is_reference = 0) const;
        const char* property_declaration(uint_t index, bool include_bamespace = false) const;

        // Behaviours
        uint_t behaviour_count() const;
        ScriptFunction behaviour_by_index(uint_t index, ScriptClassBehave* behaviour = nullptr) const;

        // Child types
        uint_t child_funcdef_count();
        ScriptTypeInfo child_funcdef(uint_t index) const;
        ScriptTypeInfo parent_type() const;

        // Enums
        uint_t enum_value_count() const;
        const char* enum_value_by_index(uint_t index, int_t* out_value) const;

        // Typedef
        int_t typedef_type_id() const;

        // Funcdef
        ScriptFunction funcdef_signature() const;

        // Flags processing
        bool is_enum() const;

        ~ScriptTypeInfo();
        friend class ScriptModule;
        friend class ScriptFunction;
        friend class ScriptEngine;
        friend class ScriptObject;
    };
}// namespace Engine
