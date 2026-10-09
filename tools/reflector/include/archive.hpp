#pragma once

#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace Reflector
{
	template<typename T>
	concept VectorLike = requires(T& value, typename T::size_type size) {
		typename T::value_type;
		typename T::size_type;

		{ value.data() } -> std::same_as<typename T::value_type*>;
		{ value.size() } -> std::convertible_to<std::size_t>;
		{ value.resize(size) } -> std::same_as<void>;
	};


	class Archive
	{
	private:
		std::ostream* m_out = nullptr;
		std::istream* m_in  = nullptr;

		template<class>
		static constexpr bool always_false = false;

		template<class T>
		void process(T& value)
		{
			if constexpr (requires(T& x, Archive& ar) { x.serialize(ar); })
			{
				value.serialize(*this);
			}
			else if constexpr (requires(T& x, Archive& ar) { serialize(ar, x); })
			{
				serialize(*this, value);
			}
			else if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>)
			{
				memory(&value, sizeof(value));
			}
			else
			{
				static_assert(always_false<T>, "No serializer found for type");
			}
		}

	public:
		explicit Archive(std::ostream& stream) : m_out(&stream) {}

		explicit Archive(std::istream& stream) : m_in(&stream) {}

		[[nodiscard]]
		inline std::istream* reader() const noexcept
		{
			return m_in;
		}

		[[nodiscard]]
		inline std::ostream* writer() const noexcept
		{
			return m_out;
		}

		inline void memory(void* data, std::size_t size)
		{
			if (m_out)
			{
				m_out->write(static_cast<const char*>(data), size);

				if (!*m_out)
					throw std::runtime_error("Write failed");
			}
			else
			{
				m_in->read(static_cast<char*>(data), size);

				if (!*m_in)
					throw std::runtime_error("Read failed");
			}
		}

		template<typename T>
		T load()
		{
			if (reader())
			{
				T tmp;
				process(tmp);
				return tmp;
			}
			return T();
		}

		template<typename T>
		void store(const T& value)
		{
			if (writer())
			{
				T tmp = value;
				process(tmp);
			}
		}

		template<class... Ts>
		void operator()(Ts&... values)
		{
			(process(values), ...);
		}
	};

	template<VectorLike T>
	void serialize(Archive& ar, T& value)
	{
		using Element = typename T::value_type;
		auto size     = value.size();

		ar(size);

		if (ar.reader())
		{
			value.resize(size);
		}

		if constexpr (std::is_arithmetic_v<Element> || std::is_enum_v<Element>)
		{
			if (size > 0)
			{
				ar.memory(value.data(), size * sizeof(Element));
			}
		}
		else
		{
			for (auto& element : value)
			{
				ar(element);
			}
		}
	}
}// namespace Reflector
