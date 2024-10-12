#include <Core/engine_types.hpp>
#include <Core/etl/type_info.hpp>

namespace Engine
{
	std::size_t type_info_base::generate_id(const char* name)
	{
		static Set<StringView> bindings;
		static auto conv = [](const char* func_name) -> std::size_t { return reinterpret_cast<std::size_t>(func_name); };

		auto it = bindings.find(name);
		if (it != bindings.end())
			return conv(it->data());

		bindings.insert(name);
		return conv(name);
	}
}// namespace Engine
