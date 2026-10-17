#pragma once

namespace Trinex::Refl::New
{
	class Type;
	class Enum;
	class Struct;
	class Class;
	class Function;
	class Property;

	template<typename T>
	Type* type_of();

	template<typename T>
	Enum* enum_of();

	template<typename T>
	Struct* struct_of();

	template<typename T>
	Class* class_of();

	template<auto value>
	Function* function_of();

	template<auto value>
	Property* property_of();
}// namespace Trinex::Refl::New
