#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/directory_iterator.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
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

			if (name == "scripts:" && m_name == "scripts:")
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
		static TreeSet<String> parse_metadata(const std::map<int, std::vector<std::string>>& in_map, int type_id)
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

		static TreeMap<int_t, TreeSet<String>> parse_metadata(const std::map<int, std::vector<std::string>>& in_map)
		{
			TreeMap<int_t, TreeSet<String>> result;

			for (auto& [type_id, metadata] : in_map)
			{
				result[type_id] = parse_metadata(in_map, type_id);
			}

			return result;
		}

		TreeMap<int_t, TreeSet<String>> type_metadata_map() const
		{
			return parse_metadata(typeMetadataMap);
		}

		TreeMap<int_t, TreeSet<String>> func_metadata_map()
		{
			return parse_metadata(funcMetadataMap);
		}

		TreeMap<int_t, TreeSet<String>> var_metadata_map()
		{
			return parse_metadata(varMetadataMap);
		}

		TreeMap<int_t, Script::ClassMetadata> class_metadata_map()
		{
			TreeMap<int_t, Script::ClassMetadata> result;

			for (auto& [type_id, metadata] : classMetadataMap)
			{
				auto info = ScriptEngine::type_info_by_id(type_id);
				if (!info.is_valid())
					continue;

				auto& result_metadata			  = result[type_id];
				result_metadata.type_info		  = info;
				result_metadata.class_metadata	  = parse_metadata(typeMetadataMap, type_id);
				result_metadata.func_metadata_map = parse_metadata(metadata.funcMetadataMap);
				result_metadata.prop_metadata_map = parse_metadata(metadata.varMetadataMap);
			}
			return result;
		}
	};

	Script::Script(ScriptFolder* folder, const String& name)
		: m_path(folder->path() / name), m_name(name), m_folder(folder), m_is_dirty(false)
	{}

	Script::~Script()
	{
		unload_classes();
		m_module.discard();
		m_folder->m_scripts.erase(m_name);
	}

	Script& Script::on_path_changed()
	{
		m_path = m_folder->path() / m_name;
		return *this;
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
		m_code	   = code;
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
		m_func_metadata_map	 = builder.func_metadata_map();
		m_var_metadata_map	 = builder.var_metadata_map();
		m_class_metadata_map = builder.class_metadata_map();
		return *this;
	}

	static Object* script_object_constructor(Class* self, StringView name, Object* owner)
	{
		asITypeInfo* type = self->script_type_info.info();

		asIScriptFunction* factory = nullptr;
		{
			auto factories = type->GetFactoryCount();
			for (asUINT i = 0; i < factories; ++i)
			{
				auto current_factory = type->GetFactoryByIndex(i);
				if (current_factory->GetParamCount() == 0)
				{
					factory = current_factory;
					break;
				}
			}

			if (factory == nullptr)
			{
				throw EngineException("The script class does not contain a default constructor");
			}
		}
		
		auto obj = ScriptContext::execute(factory).address_as<Object>();
		
		if(obj == nullptr)
		{
			throw EngineException("Failed to create new instance");
		}
		
		if (!name.empty() || owner)
		{
			obj->rename(name, owner);
		}

		return obj;
	}

	static void script_object_destructor(Object* object)
	{
		auto script_object = reinterpret_cast<asIScriptObject*>(object);
		script_object->Destroy();
		std::destroy_at(object);
		
		script_object->FreeObjectMemory();
	}

	bool Script::load_classes(asITypeInfo* info)
	{
		if (info == nullptr)
			return false;

		if (info->GetNativeClassUserData())
			return true;

		auto base = info->GetBaseType();

		if (base == nullptr)
			return (info->GetFlags() & asOBJ_APP_NATIVE) != 0;

		if (!load_classes(base))
			return false;

		auto base_class = reinterpret_cast<Class*>(base->GetNativeClassUserData());

		auto full_name = Strings::concat_scoped_name(Strings::make_string_view(info->GetNamespace()),
													 Strings::make_string_view(info->GetName()));

		Class* script_class = new Class(full_name, base_class, Class::IsScriptable);

		info->SetNativeClassUserData(script_class);

		script_class->script_type_info = info;
		script_class->static_constructor(script_object_constructor);
		script_class->destroy_func(script_object_destructor);


		m_classes.insert(script_class);
		script_class->on_class_destroy.push([this](Class* self) { m_classes.erase(self); });

		return true;
	}

	Script& Script::load_classes()
	{
		auto count = m_module.object_type_count();

		for (uint_t i = 0; i < count; ++i)
		{
			auto type = m_module.object_type_by_index(i);
			load_classes(type.info());
		}

		return *this;
	}

	Script& Script::unload_classes()
	{
		auto classes = std::move(m_classes);

		for (Class* class_instance : classes)
		{
			delete class_instance;
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

		unload_classes();

		if (m_module.is_valid())
		{
			on_discard(this);
			m_module.discard();
		}

		m_module = builder.GetModule();
		m_module.name(path().str());
		m_module.as_module()->SetUserData(this, Constants::script_userdata_id);
		load_metadata(builder);
		load_classes();

		on_build(this);
		return true;
	}

	// Metadata

	const TreeMap<int_t, TreeSet<String>>& Script::func_metadata_map() const
	{
		return m_func_metadata_map;
	}

	const TreeMap<int_t, TreeSet<String>>& Script::var_metadata_map() const
	{
		return m_var_metadata_map;
	}

	const TreeMap<int_t, Script::ClassMetadata>& Script::class_metadata_map() const
	{
		return m_class_metadata_map;
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_func(const ScriptFunction& func) const
	{
		auto it = func_metadata_map.find(func.id());
		if (it != func_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::ClassMetadata::metadata_for_var(uint_t prop_index) const
	{
		auto it = prop_metadata_map.find(prop_index);
		if (it != prop_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::metadata_for_func(const ScriptFunction& func) const
	{
		auto it = m_func_metadata_map.find(func.id());
		if (it != m_func_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const TreeSet<String>& Script::metadata_for_var(uint_t var_index) const
	{
		auto it = m_var_metadata_map.find(var_index);
		if (it != m_var_metadata_map.end())
			return it->second;
		return default_value_of<TreeSet<String>>();
	}

	const Script::ClassMetadata& Script::metadata_for_class(const ScriptTypeInfo& info) const
	{
		auto it = m_class_metadata_map.find(info.type_id());
		if (it != m_class_metadata_map.end())
			return it->second;
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
