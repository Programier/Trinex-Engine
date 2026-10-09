#pragma once
#include <iosfwd>
#include <memory>

namespace Reflector
{
	std::unique_ptr<std::ostream> code_stream(std::ostream& stream);

	std::ostream& indent(std::ostream& stream);
	std::ostream& unindent(std::ostream& stream);
}// namespace Reflector
