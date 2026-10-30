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
	using DynamicFunction = void (*)();
	using DynamicInvoker  = void (*)(void*, const Refl::Function*);

	struct EntryFlags
	{
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

	struct ExecutionFlags
	{
		enum Enum : u32
		{
			Undefined     = 0,
			IgnoreUnknown = 1 << 0,
		};

		trinex_bitfield_enum_struct(ExecutionFlags, u32);
	};

	trinex_api DynamicFunction resolve_function(const Refl::Function* function);

	trinex_api void variable(StringView name, void* address, Refl::Type* reflection, StringView description = "",
	                         EntryFlags flags = EntryFlags::Undefined);

	trinex_api void function(StringView name, Refl::Function* function, StringView description = "",
	                         EntryFlags flags = EntryFlags::Undefined);

	trinex_api void function(StringView name, DynamicFunction function, DynamicInvoker invoker, Refl::Type* result,
	                         Refl::Type** args, usize count, StringView description = "",
	                         EntryFlags flags = EntryFlags::Undefined);

	trinex_api void unbind(StringView name);

	trinex_api DynamicValue execute(StringView source, ExecutionFlags flags = {});
	trinex_api DynamicValue execute_config(const Path& path);

	template<typename T>
	inline void variable(StringView name, T* address, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
	{
		::Trinex::Console::variable(name, static_cast<void*>(address), Refl::type_of<T>(), description, flags);
	}

	template<auto func>
	inline void function(StringView name, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
	{
		::Trinex::Console::function(name, Refl::function_of<func>(), description, flags);
	}

	template<typename Ret, typename... Args>
	inline void function(StringView name, Ret (*func)(Args...), StringView description = "",
	                     EntryFlags flags = EntryFlags::Undefined)
	{
		constexpr auto resolver = +[](void*, const Refl::Function* function) {
			return reinterpret_cast<Ret (*)(Args...)>(resolve_function(function));
		};

		auto address = reinterpret_cast<DynamicFunction>(func);
		auto invoker = etl::dynamic_invoker<resolver, const Refl::Function*>;

		if constexpr (sizeof...(Args) == 0)
		{
			function(name, address, invoker, Refl::type_of<Ret>(), nullptr, 0, description, flags);
		}
		else
		{
			Refl::Type* types[] = {Refl::type_of<Args>()...};
			function(name, address, invoker, Refl::type_of<Ret>(), types, sizeof...(Args), description, flags);
		}
	}

	template<typename T>
	class Variable final
	{
	public:
		struct VariableInfo
		{
			T value;
			StringView name;
			StringView description;
			EntryFlags flags;
		};

	private:
		String m_name;
		T m_value;
		EntryFlags m_flags;

		inline void bind(StringView description) { Console::variable(m_name, &m_value, description, m_flags); }

	public:
		using ValueType = T;

		Variable(StringView name, const T& value, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
		    : m_name(name), m_value(value), m_flags(flags)
		{
			bind(description);
		}

		Variable(StringView name, T&& value, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
		    : m_name(name), m_value(static_cast<T&&>(value)), m_flags(flags)
		{
			bind(description);
		}

		~Variable() { Console::unbind(m_name); }

		trinex_non_copyable(Variable);
		trinex_non_moveable(Variable);

		inline StringView name() const { return m_name; }
		inline EntryFlags flags() const { return m_flags; }

		inline T& value() { return m_value; }
		inline const T& value() const { return m_value; }
		inline T* data() { return &m_value; }
		inline const T* data() const { return &m_value; }

		inline void value(const T& new_value) { m_value = new_value; }
		inline void value(T&& new_value) { m_value = static_cast<T&&>(new_value); }

		inline Variable& operator=(const T& new_value)
		{
			value(new_value);
			return *this;
		}

		inline Variable& operator=(T&& new_value)
		{
			value(static_cast<T&&>(new_value));
			return *this;
		}

		inline operator T&() { return m_value; }
		inline operator const T&() const { return m_value; }
		inline T& operator*() { return m_value; }
		inline const T& operator*() const { return m_value; }
		inline T* operator->() { return &m_value; }
		inline const T* operator->() const { return &m_value; }
	};
} // namespace Trinex::Console

#define trinex_console_var(Type, variable, ...) ::Trinex::Console::Variable<Type> variable(__VA_ARGS__)
