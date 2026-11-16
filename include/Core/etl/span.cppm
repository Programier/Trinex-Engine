module;
#include <span>

export module Trinex.Core.ETL.Span;

export namespace Trinex
{
	template<typename Type, std::size_t extend = std::dynamic_extent>
	using Span = std::span<Type, extend>;
}
