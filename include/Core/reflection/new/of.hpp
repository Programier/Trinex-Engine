#pragma once

namespace Trinex::Refl::New
{
	class Type;
	class Enum;
	class Struct;
	class Class;
	class Function;
	class Property;

	// clang-format off
	template<typename T> struct reflected_type { using type = T; };
	template<typename T> struct reflected_type<T&> : reflected_type<T> {};
	template<typename T> struct reflected_type<T&&> : reflected_type<T> {};
	template<typename T> struct reflected_type<const T> : reflected_type<T> {};
	template<typename T> struct reflected_type<volatile T> : reflected_type<T> {};
	template<typename T> struct reflected_type<const volatile T> : reflected_type<T> {};
	template<typename T> struct reflected_type<T*> { using type = T; };
	template<typename T> struct reflected_type<const T*> { using type = T; };
	template<typename T> struct reflected_type<volatile T*> { using type = T; };
	template<typename T> struct reflected_type<const volatile T*> { using type = T; };
	// clang-format on

	template<typename T>
	struct TypeOf {
		static Type* resolve();
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
