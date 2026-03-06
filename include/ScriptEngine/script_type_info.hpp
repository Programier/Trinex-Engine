#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <ScriptEngine/enums.hpp>

class asITypeInfo;

namespace Engine
{
	class ScriptModule;
	class ScriptFunction;

	class ENGINE_EXPORT ScriptTypeInfo
	{
	private:
		mutable asITypeInfo* m_info = nullptr;
		const ScriptTypeInfo& add_ref() const;


	public:
		ScriptTypeInfo(asITypeInfo* info = nullptr);
		copy_constructors_hpp(ScriptTypeInfo);

		const ScriptTypeInfo& release() const;

		asITypeInfo* info() const;

		bool is_valid() const;
		// Miscellaneous
		ScriptModule module() const;

		// Type info
		StringView name() const;
		StringView namespace_name() const;
		StringView config_group() const;
		ScriptTypeInfo base_type() const;
		ScriptTypeInfo native_base_type() const;
		bool derives_from(const ScriptTypeInfo& info) const;
		i32 type_id() const;
		i32 sub_type_id(u32 index) const;
		u32 size() const;
		ScriptTypeInfo sub_type(u32 index) const;
		u32 sub_type_count() const;

		// Interfaces
		u32 interface_count() const;
		ScriptTypeInfo interface(u32 index);
		bool implements(const ScriptTypeInfo& obj_type) const;

		// Factories
		u32 factory_count() const;
		ScriptFunction factory_by_index(u32 index) const;
		ScriptFunction factory_by_decl(const char* decl) const;
		ScriptFunction factory_by_decl(const String& decl) const;

		// Methods
		u32 method_count() const;
		ScriptFunction method_by_index(u32 index, bool get_virtual = true) const;
		ScriptFunction method_by_name(const char* name, bool get_virtual = true) const;
		ScriptFunction method_by_decl(const char* decl, bool get_virtual = true) const;
		ScriptFunction method_by_name(const String& name, bool get_virtual = true) const;
		ScriptFunction method_by_decl(const String& decl, bool get_virtual = true) const;

		// Properties
		u32 property_count() const;
		bool property(u32 index, StringView* name = nullptr, i32* type_id = nullptr, bool* is_private = nullptr,
		              bool* is_protected = nullptr, i32* offset = nullptr, bool* is_reference = nullptr,
		              bool* is_const = nullptr) const;
		String property_declaration(u32 index, bool include_bamespace = false) const;
		StringView property_name(u32 index) const;
		i32 property_type_id(u32 index) const;
		i32 property_offset(u32 index) const;
		bool is_property_private(u32 index) const;
		bool is_property_protected(u32 index) const;
		bool is_property_native(u32 index) const;
		bool is_property_reference(u32 index) const;

		// Behaviours
		u32 behaviour_count() const;
		ScriptFunction behaviour_by_index(u32 index, ScriptClassBehave* behaviour = nullptr) const;

		// Child types
		u32 child_funcdef_count();
		ScriptTypeInfo child_funcdef(u32 index) const;
		ScriptTypeInfo parent_type() const;

		// Enums
		u32 enum_value_count() const;
		StringView enum_value_by_index(u32 index, i64* out_value = nullptr) const;

		// Funcdef
		ScriptFunction funcdef_signature() const;

		// Flags processing
		bool is_implicit_handle() const;
		bool is_script_object() const;
		bool is_native() const;
		bool is_native_inheritable() const;
		bool is_inheritable() const;
		bool is_registered() const;
		bool is_shared() const;
		bool is_noinherit() const;
		bool is_funcdef() const;
		bool is_template_subtype() const;
		bool is_typedef() const;
		bool is_abstract() const;
		bool is_enum() const;
		bool is_array() const;
		bool is_object(bool handle_is_object = false) const;
		bool is_handle() const;
		bool is_value() const;
		bool is_ref() const;

		bool operator==(const ScriptTypeInfo& info);
		bool operator==(const asITypeInfo* info);
		bool operator!=(const ScriptTypeInfo& info);
		bool operator!=(const asITypeInfo* info);

		~ScriptTypeInfo();
	};
}// namespace Engine
