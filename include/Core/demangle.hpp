#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>

namespace Engine::Demangle
{
	ENGINE_EXPORT String decode_name(const String& name);

	template<typename Type>
	FORCE_INLINE typename std::enable_if<is_object_based<Type>::value, String>::type name_of_type()
	{
		return Type::static_reflection()->name();
	}
}// namespace Engine::Demangle
