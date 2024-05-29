#include <Core/config_manager.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <scriptarray.h>

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
    declare_typename(int_t, int);
    declare_typename(float, float);
    declare_typename(String, string);
    declare_typename(Vector<bool>, bool[]);
    declare_typename(Vector<int_t>, int[]);
    declare_typename(Vector<float>, floatol[]);
    declare_typename(Vector<String>, string[]);


    template<typename OutType, typename InType>
    struct GetValueProxy {
        static OutType get_value_of(const void* address, int_t type_id)
        {
            return OutType();
        }
    };


#define get_value_of_func(OutType, InType, code)                                                                                 \
    template<>                                                                                                                   \
    struct GetValueProxy<OutType, InType> {                                                                                      \
        static OutType get_value_of(const void* address, int_t type_id)                                                          \
        {                                                                                                                        \
            code;                                                                                                                \
        }                                                                                                                        \
    }

    // Boolean properties
    get_value_of_func(bool, bool, return *reinterpret_cast<const bool*>(address));
    get_value_of_func(int_t, bool, return *reinterpret_cast<const bool*>(address) ? 1 : 0);
    get_value_of_func(float, bool, return *reinterpret_cast<const bool*>(address) ? 1.f : 0.f);
    get_value_of_func(String, bool, return *reinterpret_cast<const bool*>(address) ? "true" : "false");

    // Integer properties
    get_value_of_func(bool, int_t, return *reinterpret_cast<const int_t*>(address) == 0 ? false : true);
    get_value_of_func(int_t, int_t, return *reinterpret_cast<const int_t*>(address));
    get_value_of_func(float, int_t, return static_cast<float>(*reinterpret_cast<const int_t*>(address)));
    get_value_of_func(String, int_t, return Strings::format("{}", *reinterpret_cast<const int_t*>(address)));

    // Float properties
    get_value_of_func(bool, float, return *reinterpret_cast<const float*>(address) == 0.f ? false : true);
    get_value_of_func(int_t, float, return static_cast<int>(*reinterpret_cast<const float*>(address)));
    get_value_of_func(float, float, return *reinterpret_cast<const float*>(address));
    get_value_of_func(String, float, return Strings::format("{}", *reinterpret_cast<const float*>(address)));

    // String properties
    get_value_of_func(bool, String, {
        const String* line = reinterpret_cast<const String*>(address);
        return Strings::boolean_of(line->c_str(), line->length());
    });

    get_value_of_func(int_t, String, return Strings::integer_of(reinterpret_cast<const String*>(address)->c_str()));
    get_value_of_func(float, String, return Strings::float_of(reinterpret_cast<const String*>(address)->c_str()));
    get_value_of_func(String, String, return *reinterpret_cast<const String*>(address));


    // Array properties

    // Convert Any component to Array of any component

    template<typename OutComponent, typename InComponent>
    struct GetValueProxy<Vector<OutComponent>, InComponent> {
        static Vector<OutComponent> get_value_of(const void* address, int_t type_id)
        {
            return Vector<OutComponent>({GetValueProxy<OutComponent, InComponent>::get_value_of(address, type_id)});
        }
    };

    // Convert any array to integral type
    template<typename OutComponent, typename InComponentType>
    struct GetValueProxy<OutComponent, Vector<InComponentType>> {
        static OutComponent get_value_of(const void* address, int_t type_id)
        {
            return static_cast<OutComponent>(reinterpret_cast<const CScriptArray*>(address)->GetSize());
        }
    };

    // Convert any array to String
    template<typename ComponentType>
    struct GetValueProxy<String, Vector<ComponentType>> {
        static String get_value_of(const void* address, int_t type_id)
        {
            return ScriptEngine::instance()->to_string(address, type_id);
        }
    };

    // Convert any array to any array

    template<typename OutComponentType, typename InComponentType>
    struct GetValueProxy<Vector<OutComponentType>, Vector<InComponentType>> {
        static Vector<OutComponentType> get_value_of(const void* address, int_t type_id)
        {
            const CScriptArray* array = reinterpret_cast<const CScriptArray*>(address);
            Vector<OutComponentType> result(array->GetSize());

            int element_type = array->GetElementTypeId();

            for (asUINT i = 0, count = array->GetSize(); i < count; i++)
            {
                result[i] = GetValueProxy<OutComponentType, InComponentType>::get_value_of(array->At(i), element_type);
            }

            return result;
        }
    };

    static inline ScriptModule configs_module()
    {
        return ScriptModule("__TRINEX_ENGINE_CONFIGS_MODULE__");
    }

    template<typename T>
    static inline bool is_same_type(int type, const char* type_declaration)
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
        else if constexpr (std::is_same_v<T, Vector<int_t>>)
        {
            ScriptTypeInfo info = ScriptEngine::instance()->type_info_by_id(type);

            if (info.is_valid() && info.is_array())
            {
                ScriptTypeInfo sub_type_info = info.sub_type(0);

                if (sub_type_info.is_valid())
                {
                    int_t sub_type              = sub_type_info.type_id();
                    String sub_type_declaration = ScriptEngine::instance()->type_declaration(sub_type);

                    if (is_same_type<int_t>(sub_type, sub_type_declaration.c_str()))
                    {
                        return true;
                    }
                }
            }
        }

        return std::strcmp(type_declaration, type_name<T>) == 0;
    }


    template<typename T>
    static String make_variable_code(const StringView& name, const T& value)
    {
        if constexpr (std::is_same_v<T, String>)
            return Strings::format("{} {} = \"{}\";", type_name<T>, name, value);
        return Strings::format("{} {} = {};", type_name<T>, name, value);
    }

    template<typename T>
    static String make_array_code(const StringView& name, const Vector<T>& value)
    {
        if constexpr (std::is_same_v<T, String>)
            return Strings::format("array<{}> {} = {{ {} }};", type_name<T>, name,
                                   Strings::join(value, ", ", [](const T& value) { return Strings::format("\"{}\"", value); }));
        return Strings::format("array<{}> {} = {{ {} }};", type_name<T>, name, Strings::join(value, ", "));
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
                String type_declaration = ScriptEngine::instance()->type_declaration(type);

                if (!is_const && is_same_type<T>(type, type_declaration.c_str()))
                {
                    T* address = reinterpret_cast<T*>(module.address_of_global_var(variable_index));

                    if (address)
                    {
                        *address = value;
                        return true;
                    }
                }

                module.remove_global_var(variable_index);
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

        // Try to find this property
        {
            auto variable_index = module.global_var_index_by_name(namespace_name_part + String(name));

            if (variable_index >= 0)
            {
                int type      = 0;
                bool is_const = false;
                module.global_var(variable_index, nullptr, nullptr, &type, &is_const);
                String type_declaration = ScriptEngine::instance()->type_declaration(type);

                if (!is_const && is_same_type<Vector<T>>(type, type_declaration.c_str()))
                {
                    CScriptArray* array = reinterpret_cast<CScriptArray*>(module.address_of_global_var(variable_index));
                    array->Resize(value.size());

                    if (array)
                    {
                        asUINT count = array->GetSize();

                        for (asUINT i = 0; i < count; i++)
                        {
                            const T& element = value[i];
                            array->SetValue(i, const_cast<void*>(reinterpret_cast<const void*>(&element)));
                        }
                        return true;
                    }
                }

                module.remove_global_var(variable_index);
            }
        }


        String new_ns = Strings::namespace_of(name);
        new_ns        = new_ns.empty() ? namespace_name : namespace_name_part + new_ns;
        module.default_namespace(new_ns);
        String code = make_array_code(Strings::class_name_sv_of(name), value);
        int result  = module.compile_global_var(section_name, code.c_str(), 0);
        module.default_namespace(ns);
        return result >= 0;
    }

    template<typename T>
    static T get_variable_internal(const StringView& name)
    {
        ScriptModule module = configs_module();
        int variable_index  = module.global_var_index_by_name(namespace_name_part + String(name));
        if (variable_index < 0)
            return T();

        int type = 0;
        module.global_var(variable_index, nullptr, nullptr, &type);

        if (type < 0)
            return T();

        const void* address = module.address_of_global_var(variable_index);
        if (address == nullptr)
            return T();

        String type_declaration_str  = ScriptEngine::instance()->type_declaration(type);
        const char* type_declaration = type_declaration_str.c_str();

        if (is_same_type<bool>(type, type_declaration))
        {
            return GetValueProxy<T, bool>::get_value_of(address, type);
        }

        if (is_same_type<int>(type, type_declaration))
        {
            return GetValueProxy<T, int>::get_value_of(address, type);
        }

        if (is_same_type<float>(type, type_declaration))
        {
            return GetValueProxy<T, float>::get_value_of(address, type);
        }

        if (is_same_type<String>(type, type_declaration))
        {
            return GetValueProxy<T, String>::get_value_of(address, type);
        }

        if (is_same_type<Vector<bool>>(type, type_declaration))
        {
            return GetValueProxy<T, Vector<bool>>::get_value_of(address, type);
        }

        if (is_same_type<Vector<int>>(type, type_declaration))
        {
            return GetValueProxy<T, Vector<int>>::get_value_of(address, type);
        }

        if (is_same_type<Vector<float>>(type, type_declaration))
        {
            return GetValueProxy<T, Vector<float>>::get_value_of(address, type);
        }

        if (is_same_type<Vector<String>>(type, type_declaration))
        {
            return GetValueProxy<T, Vector<String>>::get_value_of(address, type);
        }

        return T();
    }

    template<typename Type>
    static void update_value_typed(const String& full_name, const void* address, int_t type)
    {
        ConfigManager::set(full_name, GetValueProxy<Type, Type>::get_value_of(address, type));
    }

    static void update_value(const String& full_name, const void* address, int_t type, const char* type_declaration)
    {
        if (is_same_type<bool>(type, type_declaration))
        {
            update_value_typed<bool>(full_name, address, type);
        }
        else if (is_same_type<int_t>(type, type_declaration))
        {
            update_value_typed<int_t>(full_name, address, type);
        }
        else if (is_same_type<float>(type, type_declaration))
        {
            update_value_typed<float>(full_name, address, type);
        }
        else if (is_same_type<String>(type, type_declaration))
        {
            update_value_typed<String>(full_name, address, type);
        }
        else if (is_same_type<Vector<bool>>(type, type_declaration))
        {
            update_value_typed<Vector<bool>>(full_name, address, type);
        }
        else if (is_same_type<Vector<int_t>>(type, type_declaration))
        {
            update_value_typed<Vector<int_t>>(full_name, address, type);
        }
        else if (is_same_type<Vector<float>>(type, type_declaration))
        {
            update_value_typed<Vector<float>>(full_name, address, type);
        }
        else if (is_same_type<Vector<String>>(type, type_declaration))
        {
            update_value_typed<Vector<String>>(full_name, address, type);
        }
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

            String type_declaration = ScriptEngine::instance()->type_declaration(type);
            update_value(full_name, address, type, type_declaration.c_str());
        }

        module.discard();
        return true;
    }

    bool ConfigManager::load_from_file(const Path& filename)
    {
        Path config_file_path = Path(get_string("Engine::configs_dir")) / filename;
        FileReader reader(config_file_path);
        if (!reader.is_open())
            return false;

        String text = reader.read_string();
        reader.close();
        if (text.empty())
            return false;
        return load_from_text(text);
    }

    void ConfigManager::initialize(bool discard)
    {
        static bool is_inited = false;

        if (is_inited && discard)
        {
            configs_module().discard();
        }

        ConfigManager::load_from_file("engine.config");
        ConfigsInitializeController().execute();

        is_inited = true;
    }

    bool ConfigManager::is_exist(const StringView& name)
    {
        return configs_module().global_var_index_by_name(namespace_name_part + String(name)) >= 0;
    }

    bool ConfigManager::set(const StringView& name, bool value)
    {
        return set_variable_internal(name, value);
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
    {
        return set_array_internal(name, value);
    }

    bool ConfigManager::set(const StringView& name, const Vector<int>& value)
    {
        return set_array_internal(name, value);
    }

    bool ConfigManager::set(const StringView& name, const Vector<float>& value)
    {
        return set_array_internal(name, value);
    }

    bool ConfigManager::set(const StringView& name, const Vector<String>& value)
    {
        return set_array_internal(name, value);
    }

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

    Vector<bool> ConfigManager::get_bool_array(const StringView& name)
    {
        return get_variable_internal<Vector<bool>>(name);
    }

    Vector<int> ConfigManager::get_int_array(const StringView& name)
    {
        return get_variable_internal<Vector<int>>(name);
    }

    Vector<float> ConfigManager::get_float_array(const StringView& name)
    {
        return get_variable_internal<Vector<float>>(name);
    }

    Vector<String> ConfigManager::get_string_array(const StringView& name)
    {
        return get_variable_internal<Vector<String>>(name);
    }

    Vector<Path> ConfigManager::get_path_array(const StringView& name)
    {
        return {};
    }

}// namespace Engine
