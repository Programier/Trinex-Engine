#pragma once
#include <memory>

namespace Engine
{
	template<typename Type>
	using SharedPtr = std::shared_ptr<Type>;

	template<typename Type, typename Dp = std::default_delete<Type>>
	using UniquePtr = std::unique_ptr<Type, Dp>;
}// namespace Engine
