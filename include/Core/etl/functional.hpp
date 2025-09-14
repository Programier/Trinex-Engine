#pragma once
#include <functional>

namespace Engine
{
	template<typename T>
	using Less = std::less<T>;

	template<typename T>
	using Greater = std::greater<T>;

	template<typename T>
	using LessEqual = std::less_equal<T>;

	template<typename T>
	using GreaterEqual = std::greater_equal<T>;

	template<typename T>
	using EqualTo = std::equal_to<T>;

	template<typename T>
	using NotEqualTo = std::not_equal_to<T>;
}// namespace Engine
