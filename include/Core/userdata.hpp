#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/any.hpp>
#include <Core/etl/map.hpp>

namespace Engine
{
	class ENGINE_EXPORT UserData final
	{
	private:
		TreeMap<Identifier, Any> m_userdata;

	public:
		UserData& clear();
		UserData& remove(Identifier id);
		UserData& remove(const void* id);

		Any& get(Identifier id);
		Any& get(const void* id);
	};
}// namespace Engine
