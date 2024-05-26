#include <Core/config_manager.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
    static constexpr inline const char* section_name        = "__TRINEX_CONFIGS__";
    static constexpr inline const char* namespace_name      = "__TRINEX_CONFIGS__";
    static constexpr inline const char* namespace_name_part = "__TRINEX_CONFIGS__::";

    template<typename T>
    static constexpr inline const char* type_name = "Undefined Type";

#define declare_typename(type, name)                                                                                             \
    template<>                                                                                                                   \
    constexpr inline const char* type_name<type> = #name

    declare_typename(bool, bool);
    declare_typename(int, int);
    declare_typename(float, float);
    declare_typename(String, string);

    template<typename T, typename B>
    static T get_value_of(const void* address)
    {
        return T();
    }

#define get_value_of_func(T, B)                                                                                                  \
    template<>                                                                                                                   \
    T get_value_of<T, B>(const void* address)

    // Boolean properties

    get_value_of_func(bool, bool)
    {
        return *reinterpret_cast<const bool*>(address);
    }

    get_value_of_func(int, bool)
    {
        return *reinterpret_cast<const bool*>(address) ? 1 : 0;
    }

    get_value_of_func(float, bool)
    {
        return *reinterpret_cast<const bool*>(address) ? 1.f : 0.f;
    }

    get_value_of_func(String, bool)
    {
        return *reinterpret_cast<const bool*>(address) ? "1" : "0";
    }

    // Integer properties

    get_value_of_func(bool, int)
    {
        return *reinterpret_cast<const int*>(address) == 0 ? false : true;
    }

    get_value_of_func(int, int)
    {
        return *reinterpret_cast<const int*>(address);
    }

    get_value_of_func(float, int)
    {
        return static_cast<float>(*reinterpret_cast<const int*>(address));
    }

    get_value_of_func(String, int)
    {
        return Strings::format("{}", *reinterpret_cast<const int*>(address));
    }

    // Float properties

    get_value_of_func(bool, float)
    {
        return *reinterpret_cast<const float*>(address) == 0.f ? false : true;
    }

    get_value_of_func(int, float)
    {
        return static_cast<int>(*reinterpret_cast<const float*>(address));
    }

    get_value_of_func(float, float)
    {
        return *reinterpret_cast<const int*>(address);
    }

    get_value_of_func(String, float)
    {
        return Strings::format("{}", *reinterpret_cast<const float*>(address));
    }

    // String properties
    get_value_of_func(bool, String)
    {
        return std::stoi(*reinterpret_cast<const String*>(address)) != 0;
    }

    get_value_of_func(int, String)
    {
        return std::stoi(*reinterpret_cast<const String*>(address));
    }

    get_value_of_func(float, String)
    {
        return std::stof(*reinterpret_cast<const String*>(address));
    }

    get_value_of_func(String, String)
    {
        return *reinterpret_cast<const String*>(address);
    }

    static inline ScriptModule configs_module()
    {
        return ScriptModule("__TRINEX_ENGINE_CONFIGS_MODULE__");
    }

    template<typename T>
    static inline bool is_same_type(int type)
    {
        if (type < 0)
            return false;

        if constexpr (std::is_same_v<T, int>)
        {
            // Maybe this type is enum? Process this type as int
            ScriptTypeInfo info = ScriptEngine::instance()->type_info_by_id(type);

            if (info.is_valid() && info.is_enum())
            {
                return true;
            }
        }

        return std::strcmp(ScriptEngine::instance()->type_declaration(type), type_name<T>) == 0;
    }

    template<typename T>
    static String make_variable_code(const StringView& name, const T& value)
    {
        if constexpr (std::is_same_v<T, String>)
            return Strings::format("::{} {} = \"{}\";", type_name<T>, name, value);
        return Strings::format("{} {} = {};", type_name<T>, name, value);
    }

    template<typename T>
    static String make_array_code(const StringView& name, const Vector<T>& value)
    {
        if constexpr (std::is_same_v<T, String>)
            return Strings::format("::array<::{}> {} = {{}}", type_name<T>, name);
        return Strings::format("{} {} = {{}};", type_name<T>, name);
    }

    template<typename T>
    static bool set_variable_internal(const StringView& name, const T& value)
    {
        ScriptModule module = configs_module();
        String ns           = module.default_namespace();

        // Try to find this property
        {
            auto variable_index = module.global_var_index_by_name(namespace_name_part + String(name));

            if (variable_index >= 0)
            {
                int type      = 0;
                bool is_const = false;
                module.global_var(variable_index, nullptr, nullptr, &type, &is_const);

                if (!is_const && is_same_type<T>(type))
                {
                    T* address = reinterpret_cast<T*>(module.address_of_global_var(variable_index));

                    if (address)
                    {
                        *address = value;
                        return true;
                    }
                }
            }
        }


        String new_ns = Strings::namespace_of(name);
        new_ns        = new_ns.empty() ? namespace_name : namespace_name_part + new_ns;
        module.default_namespace(new_ns);
        String code = make_variable_code(Strings::class_name_sv_of(name), value);
        int result  = module.compile_global_var(section_name, code.c_str(), 0);
        module.default_namespace(ns);

        return result >= 0;
    }

    template<typename T>
    static bool set_array_internal(const StringView& name, const Vector<T>& value)
    {
        ScriptModule module = configs_module();
        String ns           = module.default_namespace();

        String new_ns = Strings::namespace_of(name);
        new_ns        = new_ns.empty() ? namespace_name : namespace_name_part + new_ns;
        module.default_namespace(new_ns);
        String code = make_code(Strings::class_name_sv_of(name), value);
        int result  = module.compile_global_var(section_name, code.c_str(), 0);
        module.default_namespace(ns);
        return result >= 0;
    }

    template<typename T>
    static T get_variable_internal(const StringView& _name)
    {
        String name = String(_name);

        ScriptModule module = configs_module();
        int variable_index  = module.global_var_index_by_name(namespace_name_part + name);
        if (variable_index < 0)
            return T();

        int type = 0;
        module.global_var(variable_index, nullptr, nullptr, &type);

        if (type < 0)
            return T();

        const void* address = module.address_of_global_var(variable_index);
        if (address == nullptr)
            return T();

        if (is_same_type<bool>(type))
        {
            return get_value_of<T, bool>(address);
        }

        if (is_same_type<int>(type))
        {
            return get_value_of<T, int>(address);
        }

        if (is_same_type<float>(type))
        {
            return get_value_of<T, float>(address);
        }

        if (is_same_type<String>(type))
        {
            return get_value_of<T, String>(address);
        }

        return T();
    }


    bool ConfigManager::load_from_text(const String& code)
    {
        ScriptModule module("__TRINEX_TEMPORARY_MODULE__");
        int_t result = module.add_script_section(section_name, code);
        if (result >= 0)
            result = module.build();

        if (result < 0)
        {
            module.discard();
            return false;
        }

        int_t properties_count = module.global_var_count();

        for (int_t prop_idx = 0; prop_idx < properties_count; prop_idx++)
        {
            const char* name       = nullptr;
            const char* name_space = nullptr;
            int type               = 0;

            module.global_var(prop_idx, &name, &name_space, &type);

            if (name == nullptr)
                continue;

            String full_name    = name_space ? String(name_space) + "::" + name : name;
            const void* address = module.address_of_global_var(prop_idx);

            if (address == nullptr)
                continue;

            if (is_same_type<bool>(type))
            {
                set(full_name, *reinterpret_cast<const bool*>(address));
            }
            else if (is_same_type<int>(type))
            {
                set(full_name, *reinterpret_cast<const int*>(address));
            }
            else if (is_same_type<float>(type))
            {
                set(full_name, *reinterpret_cast<const float*>(address));
            }
            else if (is_same_type<String>(type))
            {
                set(full_name, *reinterpret_cast<const String*>(address));
            }
        }

        module.discard();
        return true;
    }

    bool ConfigManager::load_from_file(const Path& filename)
    {
        Path config_file_path = Path(get_string("Engine::config_dir")) / filename;
        FileReader reader(config_file_path);
        if (!reader.is_open())
            return false;

        String text = reader.read_string();
        reader.close();
        if (text.empty())
            return false;
        return load_from_text(text);
    }

    bool ConfigManager::set(const StringView& name, int value)
    {
        return set_variable_internal(name, value);
    }

    bool ConfigManager::set(const StringView& name, float value)
    {
        return set_variable_internal(name, value);
    }

    bool ConfigManager::set(const StringView& name, const StringView& value)
    {
        return set_variable_internal(name, String(value));
    }

    bool ConfigManager::set(const StringView& name, const Vector<bool>& value)
    {}

    bool ConfigManager::set(const StringView& name, const Vector<int>& value)
    {}

    bool ConfigManager::set(const StringView& name, const Vector<float>& value)
    {}

    bool ConfigManager::set(const StringView& name, const Vector<String>& value)
    {}


    bool ConfigManager::get_bool(const StringView& name)
    {
        return get_variable_internal<bool>(name);
    }

    int ConfigManager::get_int(const StringView& name)
    {
        return get_variable_internal<int>(name);
    }

    float ConfigManager::get_float(const StringView& name)
    {
        return get_variable_internal<float>(name);
    }

    String ConfigManager::get_string(const StringView& name)
    {
        return get_variable_internal<String>(name);
    }

    Path ConfigManager::get_path(const StringView& name)
    {
        return get_string(name);
    }
}// namespace Engine
