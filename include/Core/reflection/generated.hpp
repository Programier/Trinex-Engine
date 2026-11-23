#pragma once

namespace Trinex::Refl
{
	class Type;
	class Enum;
	class Struct;
	class Class;
	class Function;
	class Property;
	
	template<typename...>
	struct TypeList;
	template<typename>
	struct Accessor;
	
	template<typename T>
	struct TypeOf
	{
		static Type* resolve();
	};
	
	template<typename T>
	struct EnumOf
	{
		static Enum* resolve();
	};
	
	template<typename T>
	struct StructOf
	{
		static Struct* resolve();
	};
	
	template<typename T>
	struct ClassOf
	{
		static Class* resolve();
	};	
} // namespace Trinex::Refl
