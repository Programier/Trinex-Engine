#pragma once
#include <model.hpp>
#include <tokenizer.hpp>

namespace Reflector
{
	// Parses source into an existing module. The caller owns module and its contents.
	bool parse(Module* module, std::string_view source);

	// Returns an owning tree on success, or nullptr on any parse error.
	Module* parse(std::string_view source, std::string_view source_name = "<memory>");
}// namespace Reflector
