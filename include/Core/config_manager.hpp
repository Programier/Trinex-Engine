#pragma once
#include <Core/arguments.hpp>
#include <Core/name.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
	using ConfigBool        = bool;
	using ConfigInt         = int_t;
	using ConfigFloat       = float;
	using ConfigString      = String;
	using ConfigBoolArray   = Vector<ConfigBool>;
	using ConfigIntArray    = Vector<ConfigInt>;
	using ConfigFloatArray  = Vector<ConfigFloat>;
	using ConfigStringArray = Vector<ConfigString>;

	struct ENGINE_EXPORT ConfigManager {
	private:
		template<typename T>
		static constexpr inline bool is_enum_type = std::is_enum_v<T> && sizeof(T) == sizeof(EnumerateType);

		template<typename T>
		struct IsEnumVector : std::false_type {
		};

		template<typename T>
		    requires(is_enum_type<T>)
		struct IsEnumVector<Vector<T>> : std::true_type {
		};

		template<typename T>
		static constexpr inline bool is_enum_vector = IsEnumVector<T>::value;


		template<typename ScriptArrayType, typename ArrayType>
		static ScriptArrayType copy_array(const ArrayType& in)
		{
			using ElementType = typename ScriptArrayType::value_type;

			ScriptArrayType out;
			out.resize(in.size());

			for (size_t i = 0, count = in.size(); i < count; ++i)
			{
				if constexpr (std::is_same_v<ElementType, bool>)
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

			return out;
		}

	public:
		static bool is_exist(const Name& name);
		static const Set<String>& groups();

		static void register_property(const Name& name, ConfigBool& property, const StringView& group);
		static void register_property(const Name& name, ConfigInt& property, const StringView& group, const char* enum_type = nullptr);
		static void register_property(const Name& name, ConfigFloat& property, const StringView& group);
		static void register_property(const Name& name, ConfigString& property, const StringView& group);
		static void register_property(const Name& name, ConfigBoolArray& property, const StringView& group);
		static void register_property(const Name& name, ConfigIntArray& property, const StringView& group, const char* enum_type = nullptr);
		static void register_property(const Name& name, ConfigFloatArray& property, const StringView& group);
		static void register_property(const Name& name, ConfigStringArray& property, const StringView& group);

		template<typename T>
		    requires(is_enum_type<T>)
		static void register_property(const Name& name, Vector<T>& prop, const StringView& group, const char* enum_type)
		{
			ConfigIntArray* array = reinterpret_cast<ConfigIntArray*>(&prop);
			register_property(name, *array, group, enum_type);
		}

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
			if constexpr (std::is_same_v<OutType, ConfigBool>)
				return bool_property(name);
			else if constexpr (std::is_same_v<OutType, ConfigInt> || is_enum_type<OutType>)
				return reinterpret_cast<OutType*>(int_property(name));
			else if constexpr (std::is_same_v<OutType, ConfigFloat>)
				return float_property(name);
			else if constexpr (std::is_same_v<OutType, ConfigString>)
				return string_property(name);
			else if constexpr (std::is_same_v<OutType, ConfigBoolArray>)
				return bool_array_property(name);
			else if constexpr (std::is_same_v<OutType, ConfigIntArray> || is_enum_vector<OutType>)
				return reinterpret_cast<OutType*>(int_array_property(name));
			else if constexpr (std::is_same_v<OutType, ConfigFloatArray>)
				return float_array_property(name);
			else if constexpr (std::is_same_v<OutType, ConfigStringArray>)
				return string_array_property(name);
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
				const Arguments::ArrayType& array = arg->get<const Arguments::ArrayType&>();

				if constexpr (std::is_same_v<Type, String>)
				{
					ConfigManager::property(arg_name, array);
				}

				else if constexpr (std::is_same_v<Type, bool>)
				{
					ConfigManager::property(arg_name, copy_array<ConfigBoolArray>(array));
				}
				else if constexpr (std::is_same_v<Type, int_t>)
				{
					ConfigManager::property(arg_name, copy_array<ConfigIntArray>(array));
				}
				else if constexpr (std::is_same_v<Type, float>)
				{
					ConfigManager::property(arg_name, copy_array<ConfigFloatArray>(array));
				}
				else
				{
					static_assert("Unsupported type!");
				}
			}
		}
	};

}// namespace Engine
