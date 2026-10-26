#pragma once
#include <Core/etl/dynamic_value.hpp>
#include <Core/etl/invoker.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	class Path;
}

namespace Trinex::Console
{
	namespace Reflection = Refl::New;

	using DynamicFunction = void (*)();
	using DynamicInvoker  = void (*)(void*, const Reflection::Function*);

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

	trinex_api DynamicFunction resolve_function(const Reflection::Function* function);

	trinex_api void property(StringView name, void* address, Reflection::Type* reflection, StringView description = "",
	                         EntryFlags flags = EntryFlags::Undefined);

	trinex_api void function(StringView name, Reflection::Function* function, StringView description = "",
	                         EntryFlags flags = EntryFlags::Undefined);

	trinex_api void function(StringView name, DynamicFunction function, DynamicInvoker invoker, Reflection::Type* result,
	                         Reflection::Type** args, usize count, StringView description = "",
	                         EntryFlags flags = EntryFlags::Undefined);

	trinex_api void unbind(StringView name);

	trinex_api DynamicValue execute(StringView source, ExecutionFlags flags = {});
	trinex_api DynamicValue execute_config(const Path& path);

	template<typename T>
	inline void property(StringView name, T* address, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
	{
		::Trinex::Console::property(name, static_cast<void*>(address), Reflection::type_of<T>(), description, flags);
	}

	template<auto func>
	inline void function(StringView name, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
	{
		::Trinex::Console::function(name, Reflection::function_of<func>(), description, flags);
	}

	template<typename Ret, typename... Args>
	inline void function(StringView name, Ret (*func)(Args...), StringView description = "",
	                     EntryFlags flags = EntryFlags::Undefined)
	{
		constexpr auto resolver = +[](void*, const Reflection::Function* function) {
			return reinterpret_cast<Ret (*)(Args...)>(resolve_function(function));
		};

		auto address = reinterpret_cast<DynamicFunction>(func);
		auto invoker = etl::dynamic_invoker<resolver, const Reflection::Function*>;

		if constexpr (sizeof...(Args) == 0)
		{
			function(name, address, invoker, Reflection::type_of<Ret>(), nullptr, 0, description, flags);
		}
		else
		{
			Reflection::Type* types[] = {Reflection::type_of<Args>()...};
			function(name, address, invoker, Reflection::type_of<Ret>(), types, sizeof...(Args), description, flags);
		}
	}
}// namespace Trinex::Console
