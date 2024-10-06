#include <Core/engine_types.hpp>
#include <Core/etl/type_id.hpp>

namespace Engine
{
	std::size_t type_id_base::generate_id(const char* func_name)
	{
		static Set<StringView> bindings;
		static auto conv = [](const char* func_name) -> std::size_t { return reinterpret_cast<std::size_t>(func_name); };

		auto it = bindings.find(func_name);
		if (it != bindings.end())
			return conv(it->data());

		bindings.insert(func_name);
		return conv(func_name);
	}
}// namespace Engine
