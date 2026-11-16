#pragma once

namespace Trinex
{
	struct type_info
	{
		const char* name;
		usize name_size;
		usize hash;
		usize size;
		usize alignment;

		constexpr bool operator==(const type_info& other) const noexcept;

		template<typename T>
		constexpr bool is() const;

		template<typename T>
		static consteval const type_info& from();
	};

	namespace type_info_detail
	{
		template<usize N>
		struct fixed_string
		{
			char data[N + 1]{};
		};

		consteval unsigned long long fnv1a(const char* name, usize length)
		{
			unsigned long long hash = 14695981039346656037ull;

			for (usize i = 0; i < length; ++i)
			{
				hash ^= static_cast<unsigned char>(name[i]);
				hash *= 1099511628211ull;
			}

			return hash;
		}

		consteval usize find(const char* str, usize size, const char* pattern, usize count, bool reverse = false)
		{
			if (count > size)
				return size;

			for (usize n = 0; n <= size - count; ++n)
			{
				usize i = reverse ? size - count - n : n;
				usize j = 0;

				while (j < count && str[i + j] == pattern[j])
					++j;

				if (j == count)
					return i;
			}

			return size;
		}

		template<typename T>
		consteval auto type_name_array()
		{
#if defined(__clang__)
			constexpr auto function = trinex_function_signature;
			constexpr char prefix[] = "[T = ";
			constexpr char suffix[] = "]";
#elif defined(__GNUC__)
			constexpr auto function = trinex_function_signature;
			constexpr char prefix[] = "with T = ";
			constexpr char suffix[] = "]";
#elif defined(_MSC_VER)
			constexpr auto function = trinex_function_signature;
			constexpr char prefix[] = "type_name_array<";
			constexpr char suffix[] = ">(void)";
#else
	#error Unsupported compiler
#endif

			constexpr usize size  = sizeof(trinex_function_signature) - 1;
			constexpr usize start = find(function, size, prefix, sizeof(prefix) - 1);
			constexpr usize end   = find(function, size, suffix, sizeof(suffix) - 1, true);

			static_assert(start < size && end < size);

			constexpr usize begin = start + sizeof(prefix) - 1;
			static_assert(begin < end);

			fixed_string<end - begin> result{};

			for (usize i = 0; i < end - begin; ++i)
				result.data[i] = function[begin + i];

			return result;
		}

		template<typename T>
		inline constexpr auto type_name_storage = type_name_array<T>();

		template<typename T>
		inline constexpr type_info info_storage = [] {
			constexpr auto& name   = type_name_storage<T>;
			constexpr usize length = sizeof(name.data) - 1;

			return type_info{name.data, length, fnv1a(name.data, length), sizeof(T), alignof(T)};
		}();
	} // namespace type_info_detail

	constexpr bool type_info::operator==(const type_info& other) const noexcept
	{
		if (hash != other.hash || name_size != other.name_size)
			return false;

		for (usize i = 0; i < name_size; ++i)
		{
			if (name[i] != other.name[i])
				return false;
		}

		return true;
	}

	template<typename T>
	constexpr bool type_info::is() const
	{
		return from<T>() == *this;
	}

	template<typename T>
	consteval const type_info& type_info::from()
	{
		return type_info_detail::info_storage<T>;
	}

} // namespace Trinex
