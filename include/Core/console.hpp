#pragma once
#include <Core/etl/dynamic_value.hpp>
#include <Core/etl/string.hpp>
#include <Core/reflection/new/of.hpp>

namespace Trinex
{
	class Path;
}

namespace Trinex::Console
{
	namespace Reflection = Refl::New;

	struct EntryFlags {
		enum Enum : u32
		{
			Undefined  = 0,
			Persistent = 1 << 0,
			ReadOnly   = 1 << 1,
			Hidden     = 1 << 2,
			Cheat      = 1 << 3,
			UI         = 1 << 4,
		};

		trinex_bitfield_enum_struct(EntryFlags, u32);
	};

	struct ExecutionFlags {
		enum Enum : u32
		{
			Undefined     = 0,
			IgnoreUnknown = 1 << 0,
		};

		trinex_bitfield_enum_struct(ExecutionFlags, u32);
	};

	trinex_api void bind(StringView name, void* address, Reflection::Type* reflection, StringView description = "",
	                     EntryFlags flags = EntryFlags::Undefined);
	trinex_api void bind(StringView name, Reflection::Function* function, StringView description = "",
	                     EntryFlags flags = EntryFlags::Undefined);

	trinex_api void unbind(StringView name);

	trinex_api DynamicValue execute(StringView source, ExecutionFlags flags = {});
	trinex_api DynamicValue execute_config(const Path& path);

	template<typename T>
	inline void bind(StringView name, T* address, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
	{
		::Trinex::Console::bind(name, static_cast<void*>(address), Reflection::type_of<T>(), description, flags);
	}

	template<auto func>
	inline void bind(StringView name, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
	{
		::Trinex::Console::bind(name, Reflection::function_of<func>(), description, flags);
	}
}// namespace Trinex::Console
