#pragma once

namespace Trinex
{
	template<typename T, typename AllocatorType>
	class Array;
}

namespace Trinex::Refl::New
{
	class Type;
	class Enum;
	class Struct;
	class Class;
	class Function;
	class Property;

	// Kept as a free functions so this header does not need to include others headers
	trinex_api Type* pointer_type_of(Type* type);
	trinex_api Type* array_type_of(Type* type);

	// clang-format off
	template<typename T> struct reflected_type { using type = T; };
	template<typename T> struct reflected_type<T&> : reflected_type<T> {};
	template<typename T> struct reflected_type<T&&> : reflected_type<T> {};
	template<typename T> struct reflected_type<const T> : reflected_type<T> {};
	template<typename T> struct reflected_type<volatile T> : reflected_type<T> {};
	template<typename T> struct reflected_type<const volatile T> : reflected_type<T> {};
	// clang-format on

	template<typename T>
	struct TypeOf {
		static Type* resolve();
	};

	template<typename T>
	struct TypeOf<T*> {
		static Type* resolve()
		{
			static Type* type = pointer_type_of(TypeOf<typename reflected_type<T>::type>::resolve());
			return type;
		}
	};

	template<typename T, typename AllocatorType>
	struct TypeOf<::Trinex::Array<T, AllocatorType>> {
		static Type* resolve()
		{
			static Type* type = array_type_of(TypeOf<typename reflected_type<T>::type>::resolve());
			return type;
		}
	};

	template<typename T>
	struct EnumOf {
		static Enum* resolve();
	};

	template<typename T>
	struct StructOf {
		static Struct* resolve();
	};

	template<typename T>
	struct ClassOf {
		static Class* resolve();
	};

	template<typename T>
	Type* type_of()
	{
		return TypeOf<typename reflected_type<T>::type>::resolve();
	}

	template<typename T>
	Enum* enum_of()
	{
		return EnumOf<typename reflected_type<T>::type>::resolve();
	}

	template<typename T>
	Struct* struct_of()
	{
		return StructOf<typename reflected_type<T>::type>::resolve();
	}

	template<typename T>
	Class* class_of()
	{
		return ClassOf<typename reflected_type<T>::type>::resolve();
	}

	template<auto value>
	Function* function_of();

	template<auto value>
	Property* property_of();
}// namespace Trinex::Refl::New
