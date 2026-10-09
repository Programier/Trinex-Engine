#pragma once
#include <model.hpp>
#include <tokenizer.hpp>

namespace Reflector
{
	// Returns an owning tree on success, or nullptr on any parse error.
	Module* parse(std::string_view source, std::string_view source_name = "<memory>");
}// namespace Reflector
