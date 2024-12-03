#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/span.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/name.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
	namespace Refl
	{
		class Object;
		class ScriptClass;
		class ScriptEnum;
		class ScriptStruct;
	}// namespace Refl

	class Script;

	class ENGINE_EXPORT ScriptFolder final
	{
	private:
		TreeMap<String, Script*> m_scripts;
		TreeMap<String, ScriptFolder*> m_folders;
		Path m_path;
		String m_name;
		ScriptFolder* m_parent = nullptr;

		ScriptFolder(const String& name, ScriptFolder* parent = nullptr);
		~ScriptFolder();

		ScriptFolder& on_path_changed();

	public:
		const TreeMap<String, Script*>& scripts() const;
		const TreeMap<String, ScriptFolder*>& sub_folders() const;
		ScriptFolder* find(const Path& path, bool create_if_not_exists = false);
		ScriptFolder* find(const Span<String>& path, bool create_if_not_exists = false);
		Script* find_script(const Path& script_path, bool create_if_not_exists = false);
		Script* find_script(const Span<String>& path, bool create_if_not_exists = false);

		ScriptFolder* parent() const;
		const String& name() const;
		const Path& path() const;

		friend class ScriptEngine;
		friend class Script;
	};

	class ENGINE_EXPORT Script final
	{
	public:
		struct ENGINE_EXPORT ClassMetadata {
			ScriptTypeInfo type_info;
			TreeSet<String> class_metadata;
			TreeMap<int_t, TreeSet<String>> func_metadata_map;
			TreeMap<String, TreeSet<String>> prop_metadata_map;

			const TreeSet<String>& metadata_for_func(const ScriptFunction& func) const;
			const TreeSet<String>& metadata_for_func(const char* decl, bool get_virtual = true) const;
			const TreeSet<String>& metadata_for_func(int_t func_id) const;

			const TreeSet<String>& metadata_for_property(uint_t prop_index) const;
			const TreeSet<String>& metadata_for_property(const String& name) const;
		};

	private:
		class Builder;

		Path m_path;
		ScriptModule m_module;
		String m_name;
		String m_code;
		ScriptFolder* m_folder;
		Set<Refl::Object*> m_refl_objects;

		// Metadata info
		TreeMap<int_t, TreeSet<String>> m_func_metadata_map;
		TreeMap<String, TreeSet<String>> m_var_metadata_map;
		TreeMap<String, ClassMetadata> m_class_metadata_map;

		mutable bool m_is_dirty;

		Script(ScriptFolder* folder, const String& name);
		Script& on_path_changed();

		Script& load_metadata(Builder& builder);
		Script& create_reflection();
		Script& delete_reflection();

	public:
		using PropertyReflectionParser = Refl::Property* (*) (Script*, Refl::Struct*, ScriptTypeInfo, uint_t);
		CallBacks<void(Script*)> on_build;
		CallBacks<void(Script*)> on_discard;
		CallBacks<void(Script*)> on_exception;

		const ScriptModule& module() const;
		const String& name() const;
		const String& code() const;
		Script& code(const String& code);

		bool is_dirty() const;
		const Path& path() const;
		bool load();
		bool save() const;
		bool build(bool exception_on_error = true);

		// Reflection generation
		Refl::Struct* create_reflection(const ScriptTypeInfo& info);
		static void register_custom_reflection_parser(StringView datatype, PropertyReflectionParser parser);

		// Metadata
		const TreeMap<int_t, TreeSet<String>>& func_metadata_map() const;
		const TreeMap<String, TreeSet<String>>& var_metadata_map() const;
		const TreeMap<String, ClassMetadata>& class_metadata_map() const;

		const TreeSet<String>& metadata_for_func(const ScriptFunction& func) const;
		const TreeSet<String>& metadata_for_func(const char* decl) const;
		const TreeSet<String>& metadata_for_func(int_t func_id) const;

		const TreeSet<String>& metadata_for_var(uint_t var_index) const;
		const TreeSet<String>& metadata_for_var(const String& name) const;

		const ClassMetadata& metadata_for_class(const ScriptTypeInfo& info) const;
		const ClassMetadata& metadata_for_class(const String& name) const;
		const ClassMetadata& metadata_for_class(int_t type_id) const;

		// Functions
		uint_t functions_count() const;
		ScriptFunction function_by_index(uint_t index) const;
		ScriptFunction function_by_decl(const char* decl) const;
		ScriptFunction function_by_name(const char* name) const;
		ScriptFunction function_by_decl(const String& decl) const;
		ScriptFunction function_by_name(const String& name) const;

		// Global variables
		uint_t global_var_count() const;
		int_t global_var_index_by_name(const char* name) const;
		int_t global_var_index_by_decl(const char* decl) const;
		int_t global_var_index_by_name(const String& name) const;
		int_t global_var_index_by_decl(const String& decl) const;
		bool global_var(uint_t index, StringView* name = nullptr, StringView* name_space = nullptr, int_t* type_id = nullptr,
		                bool* is_const = nullptr) const;
		String global_var_declaration(uint_t index, bool include_namespace = false) const;
		void* address_of_global_var(uint_t index);

		// Type identification
		uint_t object_type_count() const;
		ScriptTypeInfo object_type_by_index(uint_t index) const;
		int_t type_id_by_decl(const char* decl) const;
		int_t type_id_by_decl(const String& decl) const;
		ScriptTypeInfo type_info_by_name(const char* name) const;
		ScriptTypeInfo type_info_by_decl(const char* decl) const;
		ScriptTypeInfo type_info_by_name(const String& name) const;
		ScriptTypeInfo type_info_by_decl(const String& decl) const;

		// Enums
		uint_t enum_count() const;
		ScriptTypeInfo enum_by_index(uint_t index) const;

		// Typedefs
		uint_t typedef_count() const;
		ScriptTypeInfo typedef_by_index(uint_t index) const;

		~Script();
		friend class ScriptFolder;
		friend class Refl::ScriptClass;
		friend class Refl::ScriptStruct;
		friend class Refl::ScriptEnum;
	};
}// namespace Engine
