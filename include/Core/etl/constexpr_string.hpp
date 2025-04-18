#pragma once
#include <Core/engine_types.hpp>
#include <algorithm>

namespace Engine
{
	template<size_t N>
	struct ConstexprString {
		char data[N]{};

		consteval ConstexprString(const char (&str)[N]) { std::copy_n(str, N, data); }

		consteval bool operator==(const ConstexprString<N> str) const { return std::equal(str.data, str.data + N, data); }

		template<size_t N2>
		consteval bool operator==(const ConstexprString<N2> s) const
		{
			return false;
		}

		template<size_t N2>
		consteval ConstexprString<N + N2 - 1> operator+(const ConstexprString<N2> str) const
		{
			char newchar[N + N2 - 1]{};
			std::copy_n(data, N - 1, newchar);
			std::copy_n(str.data, N2, newchar + N - 1);
			return newchar;
		}

		consteval char operator[](size_t n) const { return data[n]; }

		consteval size_t size() const { return N - 1; }

		consteval const char* c_str() const { return data; }
	};

	template<size_t s1, size_t s2>
	consteval auto operator+(ConstexprString<s1> fs, const char (&str)[s2])
	{
		return fs + ConstexprString<s2>(str);
	}

	template<size_t s1, size_t s2>
	consteval auto operator+(const char (&str)[s2], ConstexprString<s1> fs)
	{
		return ConstexprString<s2>(str) + fs;
	}

	template<size_t s1, size_t s2>
	consteval auto operator==(ConstexprString<s1> fs, const char (&str)[s2])
	{
		return fs == ConstexprString<s2>(str);
	}

	template<size_t s1, size_t s2>
	consteval auto operator==(const char (&str)[s2], ConstexprString<s1> fs)
	{
		return ConstexprString<s2>(str) == fs;
	}
}// namespace Engine
