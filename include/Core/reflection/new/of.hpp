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
	const Type* type_of();

	template<typename T>
	const Enum* enum_of();

	template<typename T>
	const Struct* struct_of();

	template<typename T>
	const Class* class_of();

	template<auto value>
	const Function* function_of();

	template<auto value>
	const Property* property_of();
}// namespace Trinex::Refl::New
