#include <Core/math/vector.hpp>
#include <Core/reflection/native.hpp>
#include <Core/reflection/of.hpp>
#include <Core/reflection/type.hpp>

namespace Trinex::Refl
{
	namespace Native
	{
		template<typename T, typename Value>
		struct NumericFormatter
		{
			static usize parse(const Type*, void* self, StringView value)
			{
				Value parsed = {};

				if (usize count = Strings::parse(value, parsed))
				{
					*static_cast<T*>(self) = static_cast<T>(parsed);
					return count;
				}

				return 0;
			}

			static bool format(const Type*, const void* self, String& value)
			{
				return Strings::format(value, static_cast<Value>(*static_cast<const T*>(self)));
			}
		};

		template<>
		struct TypeFormatter<bool>
		{
			static usize parse(const Type*, void* self, StringView value)
			{
				bool parsed = false;

				if (usize count = Strings::parse(value, parsed))
				{
					*static_cast<bool*>(self) = parsed;
					return count;
				}

				return 0;
			}

			static bool format(const Type*, const void* self, String& value)
			{
				return Strings::format(value, *static_cast<const bool*>(self));
			}
		};

		template<>
		struct TypeFormatter<String>
		{
			static usize parse(const Type*, void* self, StringView value)
			{
				const usize size = value.size();

				if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.front() == value.back())
				{
					String result;
					result.reserve(value.size() - 2);

					for (usize index = 1; index + 1 < value.size(); ++index)
					{
						char character = value[index];

						if (character == '\\' && index + 2 < value.size())
						{
							character = value[++index];

							switch (character)
							{
								case 'n': character = '\n'; break;
								case 'r': character = '\r'; break;
								case 't': character = '\t'; break;
								default: break;
							}
						}

						result += character;
					}

					*static_cast<String*>(self) = etl::move(result);
					return size;
				}

				*static_cast<String*>(self) = value;
				return size;
			}

			static bool format(const Type*, const void* self, String& value)
			{
				value += '"';

				for (char character : *static_cast<const String*>(self))
				{
					switch (character)
					{
						case '\\': value += "\\\\"; break;
						case '"': value += "\\\""; break;
						case '\n': value += "\\n"; break;
						case '\r': value += "\\r"; break;
						case '\t': value += "\\t"; break;
						default: value += character; break;
					}
				}

				value += '"';
				return true;
			}
		};

		template<>
		struct TypeFormatter<Vector2u>
		{
			static usize parse(const Type*, void* self, StringView value)
			{
				if (value.size() < 5 || value[0] != '[' || value[value.size() - 1] != ']')
					return 0;

				const usize separator = value.find(',');
				if (separator == StringView::npos)
					return 0;

				u64 x             = 0;
				u64 y             = 0;
				StringView first  = value.substr(1, separator - 1);
				StringView second = value.substr(separator + 1, value.size() - separator - 2);

				auto trim = [](StringView& text) {
					while (!text.empty() && (text.front() == ' ' || text.front() == '\t'))
						text = text.substr(1);
					while (!text.empty() && (text.back() == ' ' || text.back() == '\t'))
						text = text.substr(0, text.size() - 1);
				};

				trim(first);
				trim(second);

				if (Strings::parse(first, x) != first.size() || Strings::parse(second, y) != second.size())
					return 0;

				*static_cast<Vector2u*>(self) = {static_cast<u32>(x), static_cast<u32>(y)};
				return value.size();
			}

			static bool format(const Type*, const void* self, String& value)
			{
				const Vector2u& vector = *static_cast<const Vector2u*>(self);
				value += '[';
				Strings::format(value, static_cast<u64>(vector.x));
				value += ", ";
				Strings::format(value, static_cast<u64>(vector.y));
				value += ']';
				return true;
			}
		};

		template<>
		struct TypeFormatter<i8> : NumericFormatter<i8, i64>
		{
		};

		template<>
		struct TypeFormatter<i16> : NumericFormatter<i16, i64>
		{
		};

		template<>
		struct TypeFormatter<i32> : NumericFormatter<i32, i64>
		{
		};

		template<>
		struct TypeFormatter<i64> : NumericFormatter<i64, i64>
		{
		};

		template<>
		struct TypeFormatter<u8> : NumericFormatter<u8, u64>
		{
		};

		template<>
		struct TypeFormatter<u16> : NumericFormatter<u16, u64>
		{
		};

		template<>
		struct TypeFormatter<u32> : NumericFormatter<u32, u64>
		{
		};

		template<>
		struct TypeFormatter<u64> : NumericFormatter<u64, u64>
		{
		};

		template<>
		struct TypeFormatter<f32> : NumericFormatter<f32, f64>
		{
		};

		template<>
		struct TypeFormatter<f64> : NumericFormatter<f64, f64>
		{
		};
	} // namespace Native

	template<typename T>
	static Type* type_of_internal(Name name)
	{
		static Native::TypeVTable<T, Type::VTable> vtable;
		static Type type(name, sizeof(T), alignof(T), 0, &vtable, Object::root());
		return &type;
	}

	template<>
	trinex_api Type* TypeOf<void>::resolve()
	{
		static Type::VTable vtable;
		static Type type("void", 0, 1, Type::IsNative | Type::IsTriviallyCopyable | Type::IsTriviallyDestructible, &vtable,
		                 Object::root());
		return &type;
	}

	template<>
	trinex_api Type* TypeOf<bool>::resolve()
	{
		return type_of_internal<bool>("bool");
	}

	template<>
	trinex_api Type* TypeOf<String>::resolve()
	{
		return type_of_internal<String>("String");
	}

	template<>
	trinex_api Type* TypeOf<Vector2u>::resolve()
	{
		return type_of_internal<Vector2u>("Vector2u");
	}

	template<>
	trinex_api Type* TypeOf<i8>::resolve()
	{
		return type_of_internal<i8>("i8");
	}

	template<>
	trinex_api Type* TypeOf<u8>::resolve()
	{
		return type_of_internal<u8>("u8");
	}

	template<>
	trinex_api Type* TypeOf<i16>::resolve()
	{
		return type_of_internal<i16>("i16");
	}

	template<>
	trinex_api Type* TypeOf<u16>::resolve()
	{
		return type_of_internal<u16>("u16");
	}

	template<>
	trinex_api Type* TypeOf<i32>::resolve()
	{
		return type_of_internal<i32>("i32");
	}

	template<>
	trinex_api Type* TypeOf<u32>::resolve()
	{
		return type_of_internal<u32>("u32");
	}

	template<>
	trinex_api Type* TypeOf<i64>::resolve()
	{
		return type_of_internal<i64>("i64");
	}

	template<>
	trinex_api Type* TypeOf<u64>::resolve()
	{
		return type_of_internal<u64>("u64");
	}

	template<>
	trinex_api Type* TypeOf<f32>::resolve()
	{
		return type_of_internal<f32>("f32");
	}

	template<>
	trinex_api Type* TypeOf<f64>::resolve()
	{
		return type_of_internal<f64>("f64");
	}

	trinex_on_register()
	{
		TypeOf<bool>::resolve();
		TypeOf<String>::resolve();
		TypeOf<Vector2u>::resolve();
		TypeOf<i8>::resolve();
		TypeOf<u8>::resolve();
		TypeOf<i16>::resolve();
		TypeOf<u16>::resolve();
		TypeOf<i32>::resolve();
		TypeOf<u32>::resolve();
		TypeOf<i64>::resolve();
		TypeOf<u64>::resolve();
		TypeOf<f32>::resolve();
		TypeOf<f64>::resolve();
	}
} // namespace Trinex::Refl
