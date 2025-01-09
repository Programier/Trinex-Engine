#pragma once
#include <Core/serializer.hpp>

namespace Engine
{
	class Archive;

	template<typename T>
	struct is_archive {
		static constexpr inline bool value = false;
	};

	template<>
	struct is_archive<Archive> {
		static constexpr inline bool value = true;
	};

	template<typename T>
	concept is_complete_archive_type = requires { is_archive<T>::value && sizeof(T); };
}// namespace Engine
