#pragma once
#include <string_view>

namespace Reflector
{
	class TranslationUnit;

	class Parser
	{
	public:
		TranslationUnit parse(std::string_view source, std::string_view source_name = "<memory>");
	};
}// namespace Reflector
