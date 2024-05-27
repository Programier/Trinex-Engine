#pragma once
#include <Core/arguments.hpp>
#include <Core/string_functions.hpp>


namespace Engine
{
    struct ENGINE_EXPORT ConfigManager {
        static bool load_from_text(const String& code);
        static bool load_from_file(const Path& filename);

        static bool is_exist(const StringView& name);
        static void initialize(bool discard = false);

        static bool set(const StringView& name, bool value);
        static bool set(const StringView& name, int value);
        static bool set(const StringView& name, float value);
        static bool set(const StringView& name, const StringView& value);

        static bool set(const StringView& name, const Vector<bool>& value);
        static bool set(const StringView& name, const Vector<int>& value);
        static bool set(const StringView& name, const Vector<float>& value);
        static bool set(const StringView& name, const Vector<String>& value);


        static bool get_bool(const StringView& name);
        static int get_int(const StringView& name);
        static float get_float(const StringView& name);
        static String get_string(const StringView& name);
        static Path get_path(const StringView& name);

        static Vector<bool> get_bool_array(const StringView& name);
        static Vector<int> get_int_array(const StringView& name);
        static Vector<float> get_float_array(const StringView& name);
        static Vector<String> get_string_array(const StringView& name);
        static Vector<Path> get_path_array(const StringView& name);


        template<typename Type>
        static void load_string_argument(const char* arg_name, const char* value_name, const Type& value)
        {
            Arguments::Argument* arg = Arguments::find(arg_name);

            if (arg && arg->type == Arguments::Type::String)
            {
                if constexpr (std::is_same_v<Type, String>)
                {
                    ConfigManager::set(value_name, arg->get<Type>());
                }
                else if constexpr (std::is_same_v<Type, bool>)
                {
                    const String& arg_value = arg->get<const String&>();
                    ConfigManager::set(value_name, Strings::boolean_of(arg_value.c_str(), arg_value.size()));
                }
                else if constexpr (std::is_same_v<Type, int_t>)
                {
                    ConfigManager::set(value_name, Strings::integer_of(arg->get<const String&>().c_str()));
                }
                else if constexpr (std::is_same_v<Type, float>)
                {
                    ConfigManager::set(value_name, Strings::float_of(arg->get<const String&>().c_str()));
                }
                else
                {
                    static_assert("Unsupported type!");
                }
            }
            else if (!ConfigManager::is_exist(value_name))
            {
                ConfigManager::set(value_name, value);
            }
        }

        template<typename Type>
        static void load_array_argument(const char* arg_name, const char* value_name, const Vector<Type>& value)
        {
            Arguments::Argument* arg = Arguments::find(arg_name);
            if (arg && arg->type == Arguments::Type::Array)
            {
                Arguments::ArrayType array = arg->get<Arguments::ArrayType>();

                if constexpr (std::is_same_v<Type, String>)
                {
                    ConfigManager::set(value_name, array);
                }
                else if constexpr (std::is_same_v<Type, bool>)
                {
                    const Arguments::ArrayType& arg_value = arg->get<const Arguments::ArrayType&>();
                    Vector<Type> result(arg_value.size());
                    std::transform(arg_value.begin(), arg_value.end(), result.begin(),
                                   [](const String& value) { return Strings::boolean_of(value.c_str(), value.size()); });
                    ConfigManager::set(value_name, result);
                }
                else if constexpr (std::is_same_v<Type, int_t>)
                {
                    const Arguments::ArrayType& arg_value = arg->get<const Arguments::ArrayType&>();
                    Vector<Type> result(arg_value.size());
                    std::transform(arg_value.begin(), arg_value.end(), result.begin(),
                                   [](const String& value) { return Strings::integer_of(value.c_str()); });
                    ConfigManager::set(value_name, result);
                }
                else if constexpr (std::is_same_v<Type, float>)
                {
                    const Arguments::ArrayType& arg_value = arg->get<const Arguments::ArrayType&>();
                    Vector<Type> result(arg_value.size());
                    std::transform(arg_value.begin(), arg_value.end(), result.begin(),
                                   [](const String& value) { return Strings::float_of(value.c_str()); });
                    ConfigManager::set(value_name, result);
                }
                else
                {
                    static_assert("Unsupported type!");
                }
            }
            else if (!ConfigManager::is_exist(value_name))
            {
                ConfigManager::set(value_name, value);
            }
        }
    };
}// namespace Engine
