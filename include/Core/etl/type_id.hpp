#pragma once
#include <Core/export.hpp>
#include <source_location>
#include <type_traits>

namespace Engine
{
	struct ENGINE_EXPORT type_id_base {
	protected:
		static std::size_t generate_id(const char* func_name);

		template<typename T>
		static inline std::size_t get_internal()
		{
			static std::size_t id = generate_id(std::source_location::current().function_name());
			return id;
		}
	};

	template<typename T>
	struct type_id : type_id_base {
		static inline std::size_t get()
		{
			return get_internal<std::decay_t<T>>();
		}

		inline operator std::size_t() const
		{
			return get();
		}
	};
}// namespace Engine
