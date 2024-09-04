#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <angelscript.h>
#include <scriptbuilder.h>

namespace Engine
{
	enum class ConfigValueType
	{
		Undefined         = 0,
		ConfigBool        = 1,
		ConfigInt         = 2,
		ConfigFloat       = 3,
		ConfigString      = 4,
		ConfigBoolArray   = 5,
		ConfigIntArray    = 6,
		ConfigFloatArray  = 7,
		ConfigStringArray = 8,
		ConfigUserType    = 9,
	};

	struct ConfigValueInfo {
		void* address                           = nullptr;
		String (*to_string_func)(void* address) = nullptr;
		String group                            = "";
		ConfigValueType type;
	};

	static Map<Name, ConfigValueInfo, Name::HashFunction>& variable_map()
	{
		static Map<Name, ConfigValueInfo, Name::HashFunction> map;
		return map;
	}

	static Set<String>& config_groups()
	{
		static Set<String> groups;
		return groups;
	}

	template<typename T>
	static constexpr const char* typename_of()
	{
		return "";
	}

	template<>
	constexpr const char* typename_of<ConfigBool>()
	{
		return "bool";
	}

	template<>
	constexpr const char* typename_of<ConfigInt>()
	{
		return "int";
	}

	template<>
	constexpr const char* typename_of<ConfigFloat>()
	{
		return "float";
	}

	template<>
	constexpr const char* typename_of<ConfigString>()
	{
		return "string";
	}

	template<>
	constexpr const char* typename_of<ConfigBoolArray>()
	{
		return "Engine::Vector<bool>";
	}

	template<>
	constexpr const char* typename_of<ConfigIntArray>()
	{
		return "Engine::Vector<int>";
	}

	template<>
	constexpr const char* typename_of<ConfigFloatArray>()
	{
		return "Engine::Vector<float>";
	}

	template<>
	constexpr const char* typename_of<ConfigStringArray>()
	{
		return "Engine::Vector<string>";
	}


	template<typename InType>
	static String convert_default_value(void* ptr)
	{
		InType* value = reinterpret_cast<InType*>(ptr);
		return Strings::to_code_string(*value);
	}

	template<typename Type>
	static ConfigValueInfo* register_property_internal(const Name& name, const char* type_declaration, void* ptr,
	                                                   const StringView& group)
	{
		if (ConfigManager::is_exist(name))
		{
			error_log("ConfigManager", "Failed to register property with name '%s'. Property is exist!", name.c_str());
			return nullptr;
		}
		String ns = Strings::namespace_of(name);
		ScriptNamespaceScopedChanger changer(ns.c_str());

		String prop_name        = Strings::class_name_of(name);
		String full_declaration = Strings::format("{} {}", type_declaration, prop_name.c_str());

		if (ScriptEngine::register_property(full_declaration.c_str(), ptr) >= 0)
		{
			auto& info   = variable_map()[name];
			info.address = ptr;
			info.group   = String(group);
			config_groups().insert(info.group);
			return &info;
		}

		return nullptr;
	}

	template<typename Type>
	static ConfigValueInfo* register_property_internal(const Name& name, Type& value, const StringView& group,
	                                                   const char* type_name = nullptr)
	{
		void* prop = nullptr;

		if constexpr (std::is_base_of_v<ScriptArrayBase, Type>)
		{
			if (!value.has_array())
			{
				value.create();
			}

			prop = value.array();
		}
		else
		{
			prop = &value;
		}

		if (type_name)
			return register_property_internal<Type>(name, type_name, prop, group);
		return register_property_internal<Type>(name, typename_of<Type>(), prop, group);
	}

	bool ConfigManager::is_exist(const Name& name)
	{
		return variable_map().contains(name);
	}

	const Set<String>& ConfigManager::groups()
	{
		return config_groups();
	}

	void ConfigManager::register_property(const Name& name, ConfigBool& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigBool>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigBool;
			info->to_string_func = convert_default_value<ConfigBool>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigInt& property, const StringView& group, const char* enum_type)
	{
		if (auto info = register_property_internal<ConfigInt>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigInt;
			info->to_string_func = convert_default_value<ConfigInt>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigFloat& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigFloat>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigFloat;
			info->to_string_func = convert_default_value<ConfigFloat>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigString& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigString>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigString;
			info->to_string_func = convert_default_value<ConfigString>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigBoolArray& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigBoolArray>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigBoolArray;
			info->to_string_func = convert_default_value<ConfigBoolArray>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigIntArray& property, const StringView& group,
	                                      const char* type_name)
	{
		if (type_name)
		{
			static String tmp;
			tmp       = Strings::format("Engine::Vector<{}>", type_name);
			type_name = tmp.c_str();
		}

		if (auto info = register_property_internal<ConfigIntArray>(name, property, group, type_name))
		{
			info->type           = ConfigValueType::ConfigIntArray;
			info->to_string_func = convert_default_value<ConfigIntArray>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigFloatArray& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigFloatArray>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigFloatArray;
			info->to_string_func = convert_default_value<ConfigFloatArray>;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigStringArray& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigStringArray>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigStringArray;
			info->to_string_func = convert_default_value<ConfigStringArray>;
		}
	}

	void ConfigManager::register_custom_property(const Name& name, void* property, const char* script_type_declaration,
	                                             const StringView& group)
	{
		if (auto info = register_property_internal<void>(name, script_type_declaration, property, group))
		{
			info->type = ConfigValueType::ConfigUserType;
		}
	}

	template<typename T>
	static bool set_property_internal(const Name& name, const T& property, const char* type_name)
	{
		T* prop = ConfigManager::property<T>(name);
		if (prop)
		{
			(*prop) = property;
			return true;
		}
		else
		{
			error_log("ConfigManager", "Cannot find property '%s' with type '%s'", name.c_str(), type_name);
		}

		return false;
	}

	template<typename T>
	static T* get_property_internal(const Name& name, ConfigValueType type)
	{
		auto& map = variable_map();
		auto it   = map.find(name);

		if (it == map.end())
			return nullptr;
		if (it->second.type == type)
		{
			return reinterpret_cast<T*>(it->second.address);
		}
		return nullptr;
	}


#define declare_setter_and_getter(type, func_prefix)                                                                             \
	bool ConfigManager::property(const Name& name, const type& property)                                                         \
	{                                                                                                                            \
		return set_property_internal<type>(name, property, #type);                                                               \
	}                                                                                                                            \
	type* ConfigManager::func_prefix##_property(const Name& name)                                                                \
	{                                                                                                                            \
		return get_property_internal<type>(name, ConfigValueType::type);                                                         \
	}

	declare_setter_and_getter(ConfigBool, bool);
	declare_setter_and_getter(ConfigInt, int);
	declare_setter_and_getter(ConfigFloat, float);
	declare_setter_and_getter(ConfigString, string);
	declare_setter_and_getter(ConfigBoolArray, bool_array);
	declare_setter_and_getter(ConfigIntArray, int_array);
	declare_setter_and_getter(ConfigFloatArray, float_array);
	declare_setter_and_getter(ConfigStringArray, string_array);

	void* ConfigManager::custom_property(const Name& name)
	{
		return get_property_internal<void>(name, ConfigValueType::ConfigUserType);
	}

	void ConfigManager::load_config_from_text(const String& config, const String& group)
	{
		if (!groups().contains(group))
		{
			error_log("ConfigManager", "Failed to find group '%s'", group.c_str());
			return;
		}

		CScriptBuilder builder;

		auto engine = ScriptEngine::engine();
		builder.StartNewModule(engine, "__TRINEX_ENGINE_ECONFIGS__");
		builder.AddSectionFromMemory("Config", config.c_str(), config.length());
		builder.BuildModule();

		auto module  = builder.GetModule();
		asUINT funcs = module->GetFunctionCount();

		const String find_entry = "config_main";
		for (asUINT i = 0; i < funcs; ++i)
		{
			auto func = module->GetFunctionByIndex(i);

			if (func->GetParamCount() == 0)
			{
				auto metadata = builder.GetMetadataForFunc(func);

				if (std::find(metadata.begin(), metadata.end(), find_entry) != metadata.end())
				{
					ScriptContext::execute(func);
				}
			}
		}

		module->Discard();
	}

	void ConfigManager::load_config_from_file(const Path& file)
	{
		FileReader reader(Path(Project::configs_dir) / file);
		if (!reader.is_open())
		{
			error_log("ConfigManager", "Failed to load config '%s'", file.c_str());
			return;
		}

		String config = reader.read_string();
		load_config_from_text(config, String(file.stem()));
	}

	void ConfigManager::initialize()
	{
		load_config_from_file("engine.config");
		ConfigsInitializeController().execute();
	}
}// namespace Engine
