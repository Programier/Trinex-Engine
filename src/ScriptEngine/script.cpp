#include <Core/constants.hpp>
#include <Core/etl/templates.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/script_class.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>
#include <scriptbuilder.h>

namespace Engine
{
	ScriptFolder::ScriptFolder(const String& name, ScriptFolder* parent) : m_name(name), m_parent(parent)
	{
		if (parent)
		{
			parent->m_folders.insert_or_assign(name, this);
			m_path = m_parent->path() / name;
		}
		else
		{
			m_path = name;
		}
	}

	ScriptFolder::~ScriptFolder()
	{
		auto childs = std::move(m_folders);

		for (auto& child : childs)
		{
			delete child.second;
		}

		if (m_parent)
		{
			m_parent->m_folders.erase(m_name);
			m_parent = nullptr;
		}

		auto scripts = std::move(m_scripts);

		for (auto& script : scripts)
		{
			delete script.second;
		}
	}

	ScriptFolder& ScriptFolder::on_path_changed()
	{
		if (m_parent)
		{
			m_path = m_parent->path() / m_name;
		}
		else
		{
			m_path = m_name;
		}

		for (auto& [name, script] : m_scripts)
		{
			script->on_path_changed();
		}

		for (auto& [name, folder] : m_folders)
		{
			folder->on_path_changed();
		}

		return *this;
	}

	const TreeMap<String, Script*>& ScriptFolder::scripts() const
	{
		return m_scripts;
	}

	const TreeMap<String, ScriptFolder*>& ScriptFolder::sub_folders() const
	{
		return m_folders;
	}

	ScriptFolder* ScriptFolder::find(const Path& path, bool create_if_not_exists)
	{
		auto splited_path = path.split();
		return find(splited_path, create_if_not_exists);
	}

	ScriptFolder* ScriptFolder::find(const Span<String>& path, bool create_if_not_exists)
	{
		ScriptFolder* folder = this;

		for (auto& name : path)
		{
			if (name.empty())
				continue;

			if (name == "[scripts_dir]:" && m_name == "[scripts_dir]:")
				continue;

			auto it = folder->m_folders.find(name);

			if (it == folder->m_folders.end())
			{
				if (create_if_not_exists)
				{
					folder = new ScriptFolder(name, folder);
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				folder = it->second;
			}
		}

		return folder;
	}

	Script* ScriptFolder::find_script(const Path& script_path, bool create_if_not_exists)
	{
		auto splited_path = script_path.split();
		return find_script(splited_path, create_if_not_exists);
	}

	Script* ScriptFolder::find_script(const Span<String>& path, bool create_if_not_exists)
	{
		if (path.empty() || !path.back().ends_with(Constants::script_extension))
			return nullptr;

		if (ScriptFolder* folder = find(path.subspan(0, path.size() - 1), create_if_not_exists))
		{
			auto it = folder->m_scripts.find(path.back());

			if (it == folder->m_scripts.end())
			{
				if (create_if_not_exists)
				{
					Script* new_script = new Script(folder, path.back());
					folder->m_scripts.insert_or_assign(path.back(), new_script);
					return new_script;
				}
				else
				{
					return nullptr;
				}
			}

			return it->second;
		}

		return nullptr;
	}

	ScriptFolder* ScriptFolder::parent() const
	{
		return m_parent;
	}

	const String& ScriptFolder::name() const
	{
		return m_name;
	}

	const Path& ScriptFolder::path() const
	{
		return m_path;
	}


	class Script::Builder : public CScriptBuilder
	{
	public:
		TreeSet<String> parse_metadata(const std::map<int, std::vector<std::string>>& in_map, int type_id)
		{
			TreeSet<String> result;

			auto it = in_map.find(type_id);
			if (it == in_map.end())
				return result;

			for (auto& ell : it->second)
			{
				result.insert(ell);
			}

			return result;
		}

		TreeMap<int_t, TreeSet<String>> parse_func_metadata(const std::map<int, std::vector<std::string>>& in_map)
		{
			TreeMap<int_t, TreeSet<String>> result;

			for (auto& [func_id, metadata] : in_map)
			{
				result[func_id] = parse_metadata(in_map, func_id);
			}

			return result;
		}

		TreeMap<String, TreeSet<String>> parse_var_metadata(const std::map<int, std::vector<std::string>>& in_map)
		{
			TreeMap<String, TreeSet<String>> result;

			for (auto& [var_idx, metadata] : in_map)
			{
				const char* name = nullptr;
				const char* ns   = nullptr;

				if (module->GetGlobalVar(var_idx, &name, &ns))
				{
					String full_name =
					        Strings::concat_scoped_name(Strings::make_string_view(ns), Strings::make_string_view(name));
					result[full_name] = parse_metadata(in_map, var_idx);
				}
			}

			return result;
		}

		TreeMap<String, TreeSet<String>> parse_prop_metadata(const ScriptTypeInfo& info,
		                                                     const std::map<int, std::vector<std::string>>& in_map)
		{
			TreeMap<String, TreeSet<String>> result;

			for (auto& [var_idx, metadata] : in_map)
			{
				String full_name  = String(info.property_name(var_idx));
				result[full_name] = parse_metadata(in_map, var_idx);
			}

			return result;
		}

		TreeMap<int_t, TreeSet<String>> func_metadata_map()
		{
			return parse_func_metadata(funcMetadataMap);
		}

		TreeMap<String, TreeSet<String>> var_metadata_map()
		{
			return parse_var_metadata(varMetadataMap);
		}

		TreeMap<String, Script::ClassMetadata> class_metadata_map()
		{
			TreeMap<String, Script::ClassMetadata> result;

			for (auto& [type_id, metadata] : typeMetadataMap)
			{
				auto info                 = ScriptEngine::type_info_by_id(type_id);
				auto& result_metadata     = result[Strings::concat_scoped_name(info.namespace_name(), info.name())];
				result_metadata.type_info = info;

				for (auto& ell : metadata)
				{
					result_metadata.class_metadata.insert(ell);
				}
			}

			for (auto& [type_id, metadata] : classMetadataMap)
			{
				auto info = ScriptEngine::type_info_by_id(type_id);
				if (!info.is_valid())
					continue;

				auto& result_metadata = result[Strings::concat_scoped_name(info.namespace_name(), info.name())];

				if (!result_metadata.type_info.is_valid())
					result_metadata.type_info = info;

				result_metadata.func_metadata_map = parse_func_metadata(metadata.funcMetadataMap);
				result_metadata.prop_metadata_map = parse_prop_metadata(info, metadata.varMetadataMap);
			}
			return result;
		}
	};

	Script::Script(ScriptFolder* folder, const String& name)
	    : m_path(folder->path() / name), m_name(name), m_folder(folder), m_is_dirty(false)
	{}

	Script::~Script()
	{
		delete_reflection();
		m_module.discard();
		on_discard.trigger(this);
		m_folder->m_scripts.erase(m_name);
	}

	Script& Script::on_path_changed()
	{
		m_path = m_folder->path() / m_name;
		return *this;
	}

	const ScriptModule& Script::module() const
	{
		return m_module;
	}

	const String& Script::name() const
	{
		return m_name;
	}

	const String& Script::code() const
	{
		return m_code;
	}

	Script& Script::code(const String& code)
	{
		m_code     = code;
		m_is_dirty = true;
		return *this;
	}

	bool Script::is_dirty() const
	{
		return m_is_dirty;
	}

	const Path& Script::path() const
	{
		return m_path;
	}

	bool Script::load()
	{
		FileReader reader(path());

		if (reader.is_open())
		{
			String new_code(reader.size(), 0);

			if (new_code.size() > 0 && reader.read(reinterpret_cast<byte*>(new_code.data()), new_code.size()))
			{
				m_code = std::move(new_code);
				return true;
			}
		}

		return false;
	}

	bool Script::save() const
	{
		FileWriter writer(path());

		if (writer.is_open())
		{
			if (writer.write(reinterpret_cast<const byte*>(m_code.c_str()), m_code.size()))
			{
				m_is_dirty = false;
				return true;
			}
		}

		return false;
	}

	Script& Script::load_metadata(Builder& builder)
	{
		m_func_metadata_map  = builder.func_metadata_map();
		m_var_metadata_map   = builder.var_metadata_map();
		m_class_metadata_map = builder.class_metadata_map();
		return *this;
	}

	static bool is_child_of_object(const ScriptTypeInfo& info)
	{
		auto p_info      = info.info();
		auto object_info = Engine::Object::static_class_instance()->script_type_info.info();

		while (p_info && p_info != object_info)
		{
			p_info = p_info->GetBaseType();
		}

		return p_info == object_info;
	}

	Script& Script::create_reflection()
	{
		auto count = m_module.object_type_count();

		for (uint_t i = 0; i < count; ++i)
		{
			auto type = m_module.object_type_by_index(i);

			if (is_child_of_object(type))
			{
				create_reflection(type.info());
			}
		}

		return *this;
	}

	Script& Script::delete_reflection()
	{
		auto refls = std::move(m_refl_objects);

		for (auto instance : refls)
		{
			Refl::Object::destroy_instance(instance);
		}

		return *this;
	}

	bool Script::build(bool exception_on_error)
	{
		Builder builder;

		const bool old_exception_on_error = ScriptEngine::exception_on_error;
		ScriptEngine::exception_on_error  = exception_on_error;

		if (builder.StartNewModule(ScriptEngine::engine(), "__TRINEX_TEMPORARY_BUILD_MODULE__") < 0)
		{
			error_log("Script", "Failed to start new module!");
			return false;
		}

		if (builder.AddSectionFromMemory(path().c_str(), m_code.data(), m_code.size()) < 0)
		{
			error_log("Script", "Failed to add script section!");
			return false;
		}

		if (builder.BuildModule() < 0)
		{
			return false;
		}

		ScriptEngine::exception_on_error = old_exception_on_error;

		delete_reflection();

		if (m_module.is_valid())
		{
			on_discard(this);
			m_module.discard();
		}

		m_module = builder.GetModule();
		m_module.name(path().str());
		m_module.as_module()->SetUserData(this, Constants::script_userdata_id);
		load_metadata(builder);
		create_reflection();

		for (auto& [id, metadata] : m_func_metadata_map)
		{
			if (metadata.contains("initializer"))
			{
				auto func = ScriptEngine::function_by_id(id);

				if (func.is_valid() && func.param_count() == 0)
				{
					ScriptContext::execute(func);
				}
			}
		}

		on_build(this);
		return true;
	}

	// Metadata

	const TreeMap<int_t, TreeSet<String>>& Script::func_metadata_map() const
	{
		return m_func_metadata_map;
	}

	const TreeMap<String, TreeSet<String>>& Script::var_metadata_map() const
	{
		return m_var_metadata_map;
	}

	const TreeMap<String, Script::ClassMetadata>& Script::class_metadata_map() const
	{
		return m_class_metadata_map;
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_func(const ScriptFunction& func) const
	{
		return metadata_for_func(func.id());
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_func(const char* decl, bool get_virtual) const
	{
		return metadata_for_func(type_info.method_by_decl(decl, get_virtual));
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_func(int_t func_id) const
	{
		auto it = func_metadata_map.find(func_id);
		if (it != func_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_property(uint_t prop_index) const
	{
		return metadata_for_property(String(type_info.property_name(prop_index)));
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_property(const String& name) const
	{
		auto it = prop_metadata_map.find(name);
		if (it != prop_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::metadata_for_func(const ScriptFunction& func) const
	{
		return metadata_for_func(func.id());
	}

	const TreeSet<String>& Script::metadata_for_func(const char* decl) const
	{
		if (m_module.is_valid())
			return metadata_for_func(m_module.function_by_decl(decl).id());
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::metadata_for_func(int_t func_id) const
	{
		auto it = m_func_metadata_map.find(func_id);
		if (it != m_func_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::metadata_for_var(uint_t var_index) const
	{
		if (m_module.is_valid())
		{
			String name = m_module.global_var_declaration(var_index, true);
			return metadata_for_var(name);
		}
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::metadata_for_var(const String& name) const
	{
		auto it = m_var_metadata_map.find(name);
		if (it != m_var_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const Script::ClassMetadata& Script::metadata_for_class(const ScriptTypeInfo& info) const
	{
		if (info.is_valid())
			return metadata_for_class(Strings::concat_scoped_name(String(info.namespace_name()), String(info.name())));
		return default_value_of<ClassMetadata>();
	}

	const Script::ClassMetadata& Script::metadata_for_class(const String& name) const
	{
		auto it = m_class_metadata_map.find(name);
		if (it != m_class_metadata_map.end())
			return it->second;
		return default_value_of<ClassMetadata>();
	}


	const Script::ClassMetadata& Script::metadata_for_class(int_t type_id) const
	{
		if (m_module.is_valid())
		{
			return metadata_for_class(ScriptEngine::type_info_by_id(type_id));
		}
		return default_value_of<ClassMetadata>();
	}

	// Functions
	uint_t Script::functions_count() const
	{
		return m_module.functions_count();
	}

	ScriptFunction Script::function_by_index(uint_t index) const
	{
		return m_module.function_by_index(index);
	}

	ScriptFunction Script::function_by_decl(const char* decl) const
	{
		return m_module.function_by_decl(decl);
	}

	ScriptFunction Script::function_by_name(const char* name) const
	{
		return m_module.function_by_name(name);
	}

	ScriptFunction Script::function_by_decl(const String& decl) const
	{
		return m_module.function_by_decl(decl);
	}

	ScriptFunction Script::function_by_name(const String& name) const
	{
		return m_module.function_by_name(name);
	}

	// Global variables
	uint_t Script::global_var_count() const
	{
		return m_module.global_var_count();
	}

	int_t Script::global_var_index_by_name(const char* name) const
	{
		return m_module.global_var_index_by_name(name);
	}

	int_t Script::global_var_index_by_decl(const char* decl) const
	{
		return m_module.global_var_index_by_decl(decl);
	}

	int_t Script::global_var_index_by_name(const String& name) const
	{
		return m_module.global_var_index_by_name(name);
	}

	int_t Script::global_var_index_by_decl(const String& decl) const
	{
		return m_module.global_var_index_by_decl(decl);
	}

	bool Script::global_var(uint_t index, StringView* name, StringView* name_space, int_t* type_id, bool* is_const) const
	{
		return m_module.global_var(index, name, name_space, type_id, is_const);
	}

	String Script::global_var_declaration(uint_t index, bool include_namespace) const
	{
		return m_module.global_var_declaration(index, include_namespace);
	}

	void* Script::address_of_global_var(uint_t index)
	{
		return m_module.address_of_global_var(index);
	}

	// Type identification
	uint_t Script::object_type_count() const
	{
		return m_module.object_type_count();
	}

	ScriptTypeInfo Script::object_type_by_index(uint_t index) const
	{
		return m_module.object_type_by_index(index);
	}

	int_t Script::type_id_by_decl(const char* decl) const
	{
		return m_module.type_id_by_decl(decl);
	}

	int_t Script::type_id_by_decl(const String& decl) const
	{
		return m_module.type_id_by_decl(decl);
	}

	ScriptTypeInfo Script::type_info_by_name(const char* name) const
	{
		return m_module.type_info_by_name(name);
	}

	ScriptTypeInfo Script::type_info_by_decl(const char* decl) const
	{
		return m_module.type_info_by_decl(decl);
	}

	ScriptTypeInfo Script::type_info_by_name(const String& name) const
	{
		return m_module.type_info_by_name(name);
	}

	ScriptTypeInfo Script::type_info_by_decl(const String& decl) const
	{
		return m_module.type_info_by_decl(decl);
	}

	// Enums
	uint_t Script::enum_count() const
	{
		return m_module.enum_count();
	}

	ScriptTypeInfo Script::enum_by_index(uint_t index) const
	{
		return m_module.enum_by_index(index);
	}

	// Typedefs
	uint_t Script::typedef_count() const
	{
		return m_module.typedef_count();
	}

	ScriptTypeInfo Script::typedef_by_index(uint_t index) const
	{
		return m_module.typedef_by_index(index);
	}

	static void static_load_scripts(ScriptFolder* folder)
	{
		auto fs = rootfs();
		for (const auto& entry : VFS::DirectoryIterator(folder->path()))
		{
			if (rootfs()->is_file(entry))
			{
				if (entry.extension() == Constants::script_extension)
				{
					auto script = folder->find_script(entry.filename(), true);
					if (script->load())
						script->build();
				}
			}
			else if (fs->is_dir(entry))
			{
				static_load_scripts(folder->find(entry.filename(), true));
			}
		}
	}

	ScriptEngine& ScriptEngine::load_scripts()
	{
		static_load_scripts(m_script_folder);
		return instance();
	}
}// namespace Engine
