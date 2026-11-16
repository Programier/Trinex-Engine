export module Trinex.Core.ETL.ConstexprString;

import Trinex.Core.Types;
import Trinex.Core.ETL.Algorithm;

export namespace Trinex
{
	template<usize N>
	struct ConstexprString
	{
		char data[N]{};

		consteval ConstexprString(const char (&str)[N]) { etl::copy_n(str, N, data); }

		consteval bool operator==(const ConstexprString<N> str) const { return etl::equal(str.data, str.data + N, data); }

		template<usize N2>
		consteval bool operator==(const ConstexprString<N2> s) const
		{
			return false;
		}

		template<usize N2>
		consteval ConstexprString<N + N2 - 1> operator+(const ConstexprString<N2> str) const
		{
			char newchar[N + N2 - 1]{};
			etl::copy_n(data, N - 1, newchar);
			etl::copy_n(str.data, N2, newchar + N - 1);
			return newchar;
		}

		consteval char operator[](usize n) const { return data[n]; }

		consteval usize size() const { return N - 1; }

		consteval const char* c_str() const { return data; }
	};

	template<usize s1, usize s2>
	consteval auto operator+(ConstexprString<s1> fs, const char (&str)[s2])
	{
		return fs + ConstexprString<s2>(str);
	}

	template<usize s1, usize s2>
	consteval auto operator+(const char (&str)[s2], ConstexprString<s1> fs)
	{
		return ConstexprString<s2>(str) + fs;
	}

	template<usize s1, usize s2>
	consteval auto operator==(ConstexprString<s1> fs, const char (&str)[s2])
	{
		return fs == ConstexprString<s2>(str);
	}

	template<usize s1, usize s2>
	consteval auto operator==(const char (&str)[s2], ConstexprString<s1> fs)
	{
		return ConstexprString<s2>(str) == fs;
	}
} // namespace Trinex
