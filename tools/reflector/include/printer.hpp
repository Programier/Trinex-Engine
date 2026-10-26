#pragma once
#include <iosfwd>

namespace Reflector
{
	class Object;
	class TypeIndex;

	void print(std::ostream& stream, const Object* object, std::size_t depth = 0);
	void print(std::ostream& stream, const TypeIndex& index);
} // namespace Reflector
