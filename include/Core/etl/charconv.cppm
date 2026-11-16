module;
#include <charconv>

export module Trinex.Core.ETL.CharConv;

export namespace Trinex::etl
{
	using std::chars_format;
	using std::from_chars;
	using std::to_chars;

	using std::from_chars_result;
	using std::to_chars_result;
} // namespace Trinex::etl
