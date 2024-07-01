#pragma once
#include <Core/arguments.hpp>
#include <Core/etl/script_array.hpp>
#include <Core/name.hpp>

namespace Engine
{
    using ConfigBool        = bool;
    using ConfigInt         = int_t;
    using ConfigFloat       = float;
    using ConfigString      = String;
    using ConfigBoolArray   = ScriptArray<ConfigBool, "bool">;
    using ConfigIntArray    = ScriptArray<ConfigInt, "int">;
    using ConfigFloatArray  = ScriptArray<ConfigFloat, "float">;
    using ConfigStringArray = ScriptArray<ConfigString, "string">;

    struct ENGINE_EXPORT ConfigManager {
    private:
        template<typename ScriptArrayType, typename ArrayType>
        static void copy_array(const ArrayType& in, ScriptArrayType& out)
        {
            using ElementType = ScriptArrayType::value_type;

            out.create(in.size());

            for (size_t i = 0, count = in.size(); i < count; ++i)
            {
                if constexpr (std::is_same_v<ElementType, String>)
                {
                    out[i] = in[i];
                }
                else if constexpr (std::is_same_v<ElementType, bool>)
                {
                    out[i] = Strings::boolean_of(in[i].c_str(), in[i].size());
                }
                else if constexpr (std::is_same_v<ElementType, int_t>)
                {
                    out[i] = Strings::integer_of(in[i].c_str());
                }
                else if constexpr (std::is_same_v<ElementType, float>)
                {
                    out[i] = Strings::float_of(in[i].c_str());
                }
            }
        }


    public:
        static bool is_exist(const Name& name);
        static const Set<String>& groups();

        static void register_property(const Name& name, ConfigBool& property, const StringView& group);
        static void register_property(const Name& name, ConfigInt& property, const StringView& group);
        static void register_property(const Name& name, ConfigFloat& property, const StringView& group);
        static void register_property(const Name& name, ConfigString& property, const StringView& group);
        static void register_property(const Name& name, ConfigBoolArray& property, const StringView& group);
        static void register_property(const Name& name, ConfigIntArray& property, const StringView& group);
        static void register_property(const Name& name, ConfigFloatArray& property, const StringView& group);
        static void register_property(const Name& name, ConfigStringArray& property, const StringView& group);
        static void register_custom_property(const Name& name, void* property, const char* script_type_declaration,
                                             const StringView& group);

        static bool property(const Name& name, const ConfigBool& property);
        static bool property(const Name& name, const ConfigInt& property);
        static bool property(const Name& name, const ConfigFloat& property);
        static bool property(const Name& name, const ConfigString& property);
        static bool property(const Name& name, const ConfigBoolArray& property);
        static bool property(const Name& name, const ConfigIntArray& property);
        static bool property(const Name& name, const ConfigFloatArray& property);
        static bool property(const Name& name, const ConfigStringArray& property);

        static ConfigBool* bool_property(const Name& name);
        static ConfigInt* int_property(const Name& name);
        static ConfigFloat* float_property(const Name& name);
        static ConfigString* string_property(const Name& name);
        static ConfigBoolArray* bool_array_property(const Name& name);
        static ConfigIntArray* int_array_property(const Name& name);
        static ConfigFloatArray* float_array_property(const Name& name);
        static ConfigStringArray* string_array_property(const Name& name);
        static void* custom_property(const Name& name);

        static void load_config_from_text(const String& config, const String& group);
        static void load_config_from_file(const Path& file);
        static void initialize();

        template<typename OutType>
        static OutType* property(const Name& name)
        {
#define wrap_config_value(code) code
            wrap_config_value(if constexpr (std::is_same_v<OutType, ConfigBool>) return bool_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigInt>) return int_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigFloat>) return float_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigString>) return string_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigBoolArray>) return bool_array_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigIntArray>) return int_array_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigFloatArray>) return float_array_property(name));
            wrap_config_value(else if constexpr (std::is_same_v<OutType, ConfigStringArray>) return string_array_property(name));
#undef wrap_config_value
            return nullptr;
        }


        template<typename Type>
        static void load_string_argument(const Name& arg_name)
        {
            Arguments::Argument* arg = Arguments::find(arg_name);

            if (arg && arg->type == Arguments::Type::String)
            {
                if constexpr (std::is_same_v<Type, String>)
                {
                    ConfigManager::property(arg_name, arg->get<Type>());
                }
                else if constexpr (std::is_same_v<Type, bool>)
                {
                    const String& arg_value = arg->get<const String&>();
                    ConfigManager::property(arg_name, Strings::boolean_of(arg_value.c_str(), arg_value.size()));
                }
                else if constexpr (std::is_same_v<Type, int_t>)
                {
                    ConfigManager::property(arg_name, Strings::integer_of(arg->get<const String&>().c_str()));
                }
                else if constexpr (std::is_same_v<Type, float>)
                {
                    ConfigManager::property(arg_name, Strings::float_of(arg->get<const String&>().c_str()));
                }
                else
                {
                    static_assert("Unsupported type!");
                }
            }
        }

        template<typename Type>
        static void load_array_argument(const Name& arg_name)
        {
            Arguments::Argument* arg = Arguments::find(arg_name);
            if (arg && arg->type == Arguments::Type::Array)
            {
                Arguments::ArrayType array = arg->get<Arguments::ArrayType>();

                if constexpr (std::is_same_v<Type, String>)
                {
                    ScriptArray<String, "string"> result_array;
                    copy_array(array, result_array);
                    ConfigManager::property(arg_name, result_array);
                }

                else if constexpr (std::is_same_v<Type, bool>)
                {
                    ScriptArray<bool, "bool"> result_array;
                    copy_array(array, result_array);
                    ConfigManager::property(arg_name, result_array);
                }
                else if constexpr (std::is_same_v<Type, int_t>)
                {
                    ScriptArray<int_t, "int"> result_array;
                    copy_array(array, result_array);
                    ConfigManager::property(arg_name, result_array);
                }
                else if constexpr (std::is_same_v<Type, float>)
                {
                    ScriptArray<float, "float"> result_array;
                    copy_array(array, result_array);
                    ConfigManager::property(arg_name, result_array);
                }
                else
                {
                    static_assert("Unsupported type!");
                }
            }
        }
    };

}// namespace Engine
