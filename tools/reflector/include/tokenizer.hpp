#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace Reflector
{
	enum class TokenType
	{
		Identifier,
		Number,
		String,
		Character,
		Symbol,
		Preprocessor,
		Comment,
		EndOfFile,
	};

	struct Token {
		TokenType type = TokenType::EndOfFile;
		std::string_view value;
		std::size_t offset = 0;
		std::size_t length = 0;
		std::size_t line   = 1;
		std::size_t column = 1;
	};

	std::vector<Token> tokenize(std::string_view source);
}// namespace Reflector
