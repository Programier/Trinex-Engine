#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <angelscript.h>

namespace Engine
{
	// Config value implementation
	struct ScriptConfigValue {
		mutable String m_cached_name;

		const String& name() const
		{
			if (m_cached_name.empty())
			{
				m_cached_name = ScriptEngine::variable_name(this);

				if (m_cached_name.starts_with("__TRINEX_ENGINE_CONFIGS_EXECUTE__::"))
				{
					m_cached_name.erase(0, 35);
				}
			}
			return m_cached_name;
		}

		template<typename T>
		ScriptConfigValue& assign_primitive(T value)
		{
			const String& parameter_name = name();
			if (!parameter_name.empty())
			{
				ConfigManager::property(parameter_name, value);
			}
			return *this;
		}

		template<typename T, ConstexprString decl>
		ScriptConfigValue& assign_array(const CScriptArray* array_base)
		{
			ScriptArray<T, decl> array;
			array.attach(const_cast<CScriptArray*>(array_base));
			assign_primitive<const ScriptArray<T, decl>&>(array);

			return *this;
		}
	};

	static void reflection_init()
	{
		using T                        = ScriptConfigValue;
		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("ConfigValue", sizeof(T));

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<T>);
		registrar.behave(ScriptClassBehave::Construct, "void f(const ConfigValue& in)",
		                 ScriptClassRegistrar::constructor<T, const T&>);
		registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<T>);

		registrar.method("const string& name() const", &T::name);
		registrar.opfunc("ConfigValue& opAssign(bool value)", &T::assign_primitive<bool>);
		registrar.opfunc("ConfigValue& opAssign(int value)", &T::assign_primitive<int_t>);
		registrar.opfunc("ConfigValue& opAssign(float value)", &T::assign_primitive<float>);
		registrar.opfunc("ConfigValue& opAssign(const string& in value)", &T::assign_primitive<const String&>);

		registrar.opfunc("ConfigValue& opAssign(const array<bool>& in value)", &T::assign_array<bool, "bool">);
		registrar.opfunc("ConfigValue& opAssign(const array<int>& in value)", &T::assign_array<int_t, "int">);
		registrar.opfunc("ConfigValue& opAssign(const array<float>& in value)", &T::assign_array<float, "float">);
		registrar.opfunc("ConfigValue& opAssign(const array<string>& in value)", &T::assign_array<String, "string">);
	}

	static ReflectionInitializeController on_reflection_init(reflection_init, "Engine::ScriptConfigValue");


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
		return "array<bool>";
	}

	template<>
	constexpr const char* typename_of<ConfigIntArray>()
	{
		return "array<int>";
	}

	template<>
	constexpr const char* typename_of<ConfigFloatArray>()
	{
		return "array<float>";
	}

	template<>
	constexpr const char* typename_of<ConfigStringArray>()
	{
		return "array<string>";
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
			info.address = ptr;// Need override for arrays!
			info.group   = String(group);
			config_groups().insert(info.group);
			return &info;
		}

		return nullptr;
	}

	template<typename Type>
	static ConfigValueInfo* register_property_internal(const Name& name, Type& value, const StringView& group)
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

	void ConfigManager::register_property(const Name& name, ConfigInt& property, const StringView& group)
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
			info->address        = &property;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigIntArray& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigIntArray>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigIntArray;
			info->to_string_func = convert_default_value<ConfigIntArray>;
			info->address        = &property;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigFloatArray& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigFloatArray>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigFloatArray;
			info->to_string_func = convert_default_value<ConfigFloatArray>;
			info->address        = &property;
		}
	}

	void ConfigManager::register_property(const Name& name, ConfigStringArray& property, const StringView& group)
	{
		if (auto info = register_property_internal<ConfigStringArray>(name, property, group))
		{
			info->type           = ConfigValueType::ConfigStringArray;
			info->to_string_func = convert_default_value<ConfigStringArray>;
			info->address        = &property;
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

	static String move_to_internal_namespace(const String& code)
	{
		return Strings::format("namespace __TRINEX_ENGINE_CONFIGS_EXECUTE__ {{ {} }}", code);
	}

	void ConfigManager::load_config_from_text(const String& config, const String& group)
	{
		if (!groups().contains(group))
		{
			error_log("ConfigManager", "Failed to find group '%s'", group.c_str());
			return;
		}
		String result_source = move_to_internal_namespace(config);
		ScriptModule module("TrinexEngineConfigs", ScriptModule::AlwaysCreate);
		module.add_script_section("Config", result_source.c_str(), result_source.size());
		module.build();
		module.discard();
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
