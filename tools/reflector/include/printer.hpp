#pragma once
#include <iosfwd>

namespace Reflector
{
	class Object;

	void print(std::ostream& stream, const Object* object, std::size_t depth = 0);
}// namespace Reflector
