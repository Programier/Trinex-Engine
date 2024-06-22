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
        ScriptEngine::NamespaceSaverScoped saver;
        String ns = Strings::namespace_of(name);
        ScriptEngine::default_namespace(ns);

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
        }
    }

    void ConfigManager::register_property(const Name& name, ConfigIntArray& property, const StringView& group)
    {
        if (auto info = register_property_internal<ConfigIntArray>(name, property, group))
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
    static void set_property_internal(const Name& name, const T& property)
    {
        T* prop = ConfigManager::property<T>(name);
        if (prop)
        {
            (*prop) = property;
        }
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
    void ConfigManager::property(const Name& name, const type& property)                                                         \
    {                                                                                                                            \
        set_property_internal<type>(name, property);                                                                             \
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


    // CONFIG LOADING

    static StringView parse_token(asIScriptEngine* engine, const char*& c_source, size_t& source_size)
    {
        asUINT token_len;
        engine->ParseToken(c_source, source_size, &token_len);
        StringView result = StringView(c_source, token_len);
        c_source += token_len;
        source_size -= token_len;
        return result;
    }

    static void return_token(const char*& c_source, size_t& source_size, StringView token)
    {
        c_source -= token.size();
        source_size += token.size();
    }

    static String create_function(const Vector<String>& expressions, int_t depth = 0)
    {
        if (expressions.empty())
            return "";

        String depth_str(depth * 4, ' ');
        String depth1_str((depth + 1) * 4, ' ');
        String depth2_str((depth + 2) * 4, ' ');

        String source =
                Strings::format("{0}class TrinexEngineConfigInitializer\n{0}{{\n{1}TrinexEngineConfigInitializer()\n{1}{{\n",
                                depth_str, depth1_str);

        for (auto& expression : expressions)
        {
            source += depth2_str;
            source += expression;
            source.push_back('\n');
        }

        source += depth1_str + "}\n";
        source += depth_str + "};\n";
        source += depth_str + "TrinexEngineConfigInitializer initializer;\n";
        return source;
    }

    static String read_config_value(asIScriptEngine* engine, const char*& c_source, size_t& source_size)
    {
        String result = "";

        while (source_size > 0)
        {
            StringView token_code = parse_token(engine, c_source, source_size);
            result += token_code;

            if (token_code == ";")
            {
                break;
            }
        }

        return result;
    }

    static void wait_end_scope(asIScriptEngine* engine, const char*& c_source, size_t& source_size, String& out)
    {
        while (source_size > 0)
        {
            StringView token = parse_token(engine, c_source, source_size);
            out += token;

            if (token == "}")
            {
                break;
            }
        }
    }

    static void process_config_namespace(asIScriptEngine* engine, const char*& c_source, size_t& source_size, String& out,
                                         int_t depth);

    static Vector<String> process_config_scope(asIScriptEngine* engine, const char*& c_source, size_t& source_size, String& out,
                                               int_t depth)
    {
        Vector<String> expressions;

        while (source_size > 0)
        {
            StringView token_code = parse_token(engine, c_source, source_size);

            if (token_code == "ConfigValue")
            {
                expressions.push_back(read_config_value(engine, c_source, source_size));
                continue;
            }
            else if (token_code == "namespace")
            {
                out += token_code;
                process_config_namespace(engine, c_source, source_size, out, depth);
                continue;
            }
            else if (token_code == "{")
            {
                out += token_code;
                process_config_scope(engine, c_source, source_size, out, depth + 1);
                wait_end_scope(engine, c_source, source_size, out);
                continue;
            }
            else if (token_code == "}")
            {
                return_token(c_source, source_size, token_code);
                break;
            }

            out += token_code;
        }

        return expressions;
    }

    static void process_config_namespace(asIScriptEngine* engine, const char*& c_source, size_t& source_size, String& out,
                                         int_t depth)
    {
        while (source_size > 0)
        {
            StringView token = parse_token(engine, c_source, source_size);
            out += token;

            if (token == "{")
            {
                break;
            }
        }

        depth += 1;

        auto expressions = process_config_scope(engine, c_source, source_size, out, depth);
        out += create_function(expressions, depth);
        wait_end_scope(engine, c_source, source_size, out);
    }

    static String parse_config(asIScriptEngine* engine, const String& source)
    {
        String result;
        const char* c_source = source.c_str();
        size_t source_len    = source.size();
        auto expressions     = process_config_scope(engine, c_source, source_len, result, 0);
        result += create_function(expressions);

        return result;
    }


    void ConfigManager::load_config_from_text(const String& config, const String& group)
    {
        if (!groups().contains(group))
        {
            error_log("ConfigManager", "Failed to find group '%s'", group.c_str());
            return;
        }
        String result_source = parse_config(ScriptEngine::as_engine(), config);
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
        for (auto& group : groups())
        {
            Path filename = group + ".config";
            load_config_from_file(filename);
        }
    }
}// namespace Engine
