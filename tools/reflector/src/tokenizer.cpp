#include "tokenizer.hpp"

#include <cctype>

namespace Reflector
{
	namespace
	{
		bool is_identifier_start(char c)
		{
			return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
		}

		bool is_identifier_body(char c)
		{
			return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
		}

		void advance_position(std::string_view source, std::size_t begin, std::size_t end, std::size_t& line, std::size_t& column)
		{
			for (std::size_t i = begin; i < end; ++i)
			{
				if (source[i] == '\n')
				{
					++line;
					column = 1;
				}
				else
				{
					++column;
				}
			}
		}

		Token make_token(TokenType type, std::string_view source, std::size_t begin, std::size_t end, std::size_t line,
		                 std::size_t column)
		{
			Token token;
			token.type   = type;
			token.value  = source.substr(begin, end - begin);
			token.offset = begin;
			token.length = end - begin;
			token.line   = line;
			token.column = column;
			return token;
		}

		std::size_t scan_quoted(std::string_view source, std::size_t offset, char quote)
		{
			bool escaped = false;
			for (std::size_t i = offset + 1; i < source.size(); ++i)
			{
				if (escaped)
				{
					escaped = false;
				}
				else if (source[i] == '\\')
				{
					escaped = true;
				}
				else if (source[i] == quote)
				{
					return i + 1;
				}
			}
			return source.size();
		}

		bool starts_with(std::string_view source, std::size_t offset, std::string_view text)
		{
			return offset + text.size() <= source.size() && source.substr(offset, text.size()) == text;
		}

	}// namespace

	std::vector<Token> tokenize(std::string_view source)
	{
		std::vector<Token> tokens;
		std::size_t line   = 1;
		std::size_t column = 1;
		std::size_t offset = 0;
		bool line_start    = true;

		while (offset < source.size())
		{
			const auto token_line   = line;
			const auto token_column = column;
			const auto c            = source[offset];

			if (std::isspace(static_cast<unsigned char>(c)))
			{
				line_start = c == '\n' ? true : line_start;
				advance_position(source, offset, offset + 1, line, column);
				++offset;
				continue;
			}

			if (line_start && c == '#')
			{
				auto end = offset + 1;
				while (end < source.size())
				{
					if (source[end] == '\n')
					{
						auto previous = end;
						if (previous > offset && source[previous - 1] == '\r')
							--previous;
						if (previous == offset || source[previous - 1] != '\\')
							break;
					}
					++end;
				}
				tokens.push_back(make_token(TokenType::Preprocessor, source, offset, end, token_line, token_column));
				advance_position(source, offset, end, line, column);
				offset     = end;
				line_start = false;
				continue;
			}

			line_start = false;

			if (starts_with(source, offset, "//"))
			{
				auto end = offset + 2;
				while (end < source.size() && source[end] != '\n')
				{
					++end;
				}
				tokens.push_back(make_token(TokenType::Comment, source, offset, end, token_line, token_column));
				advance_position(source, offset, end, line, column);
				offset = end;
				continue;
			}

			if (starts_with(source, offset, "/*"))
			{
				auto end = source.find("*/", offset + 2);
				end      = end == std::string_view::npos ? source.size() : end + 2;
				tokens.push_back(make_token(TokenType::Comment, source, offset, end, token_line, token_column));
				advance_position(source, offset, end, line, column);
				offset = end;
				continue;
			}

			if (is_identifier_start(c))
			{
				auto end = offset + 1;
				while (end < source.size() && is_identifier_body(source[end]))
				{
					++end;
				}
				tokens.push_back(make_token(TokenType::Identifier, source, offset, end, token_line, token_column));
				advance_position(source, offset, end, line, column);
				offset = end;
				continue;
			}

			if (std::isdigit(static_cast<unsigned char>(c)))
			{
				auto end = offset + 1;
				while (end < source.size() &&
				       (std::isalnum(static_cast<unsigned char>(source[end])) || source[end] == '.' || source[end] == '_'))
				{
					++end;
				}
				tokens.push_back(make_token(TokenType::Number, source, offset, end, token_line, token_column));
				advance_position(source, offset, end, line, column);
				offset = end;
				continue;
			}

			if (c == '"' || c == '\'')
			{
				const auto end = scan_quoted(source, offset, c);
				tokens.push_back(make_token(c == '"' ? TokenType::String : TokenType::Character, source, offset, end, token_line,
				                            token_column));
				advance_position(source, offset, end, line, column);
				offset = end;
				continue;
			}

			std::size_t length = 1;
			if (starts_with(source, offset, "::") || starts_with(source, offset, "&&") || starts_with(source, offset, "->") ||
			    starts_with(source, offset, "==") || starts_with(source, offset, "!=") || starts_with(source, offset, "<=") ||
			    starts_with(source, offset, ">="))
			{
				length = 2;
			}
			tokens.push_back(make_token(TokenType::Symbol, source, offset, offset + length, token_line, token_column));
			advance_position(source, offset, offset + length, line, column);
			offset += length;
		}

		Token eof;
		eof.offset = source.size();
		eof.line   = line;
		eof.column = column;
		tokens.push_back(std::move(eof));
		return tokens;
	}

}// namespace Reflector
