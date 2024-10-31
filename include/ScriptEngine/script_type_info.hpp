#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/implement.hpp>

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
		ScriptTypeInfo base_type() const;
		ScriptTypeInfo native_base_type() const;
		bool derives_from(const ScriptTypeInfo& info) const;
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
		bool property(uint_t index, StringView* name = nullptr, int_t* type_id = nullptr, bool* is_private = nullptr,
		              bool* is_protected = nullptr, int_t* offset = nullptr, bool* is_reference = nullptr) const;
		String property_declaration(uint_t index, bool include_bamespace = false) const;
		StringView property_name(uint_t index) const;
		int_t property_type_id(uint_t index) const;
		int_t property_offset(uint_t index) const;
		bool is_property_private(uint_t index) const;
		bool is_property_protected(uint_t index) const;
		bool is_property_native(uint_t index) const;
		bool is_property_reference(uint_t index) const;

		// Behaviours
		uint_t behaviour_count() const;
		ScriptFunction behaviour_by_index(uint_t index, ScriptClassBehave* behaviour = nullptr) const;

		// Child types
		uint_t child_funcdef_count();
		ScriptTypeInfo child_funcdef(uint_t index) const;
		ScriptTypeInfo parent_type() const;

		// Enums
		uint_t enum_value_count() const;
		StringView enum_value_by_index(uint_t index, int_t* out_value = nullptr) const;

		// Typedef
		int_t typedef_type_id() const;

		// Funcdef
		ScriptFunction funcdef_signature() const;

		// Flags processing
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

		~ScriptTypeInfo();
	};
}// namespace Engine
