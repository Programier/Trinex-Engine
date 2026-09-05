#pragma once
#include <Core/etl/concepts.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/span.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/tuple.hpp>
#include <Core/etl/utility.hpp>

namespace Trinex
{
	class Path;
}

namespace Trinex::Console
{
	struct EntryType {
		enum Enum : u8
		{
			Variable = 0,
			Command  = 1,
		};

		trinex_enum_struct(EntryType);
	};

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

	struct ExecuteFlags {
		enum Enum : u32
		{
			Undefined     = 0,
			IgnoreUnknown = 1 << 0,
		};

		trinex_bitfield_enum_struct(ExecuteFlags, u32);
	};

	struct ExecuteStatus {
		enum Enum : u8
		{
			Success = 0,
			EmptyInput,
			UnexpectedToken,
			UnknownEntry,
			EntryUnavailable,
			ReadOnly,
			CheatProtected,
			VariableCallSyntax,
			CommandCallSyntax,
			MissingClosingParenthesis,
			MissingRequiredParameter,
			ParameterParseFailed,
			ParameterValidationFailed,
			TooManyParameters,
			CommandHasNoCallback,
			ValueParseFailed,
			ValueValidationFailed,
			CommandFailed,
			FileOpenFailed,
		};

		trinex_enum_struct(ExecuteStatus);
	};


	struct Argument;

	template<typename T>
	concept SizeMethod =
	        requires(const T& container) { container.size(); } || requires(const T& container) { container.length(); };

	template<typename T>
	concept EmplaceBackContainer = requires(T& container) {
		typename T::value_type;
		requires SizeMethod<T>;

		container.clear();
		container.pop_back();
		{ container.emplace_back() } -> etl::same_as<typename T::value_type&>;
	};

	template<typename T>
	concept IndexedContainer = requires(T container, usize index) {
		typename T::value_type;
		requires SizeMethod<T>;

		{ container[index] } -> etl::same_as<typename T::value_type&>;
	} && !requires(T container) { container.clear(); };

	struct ExecuteContext {
		Span<Argument> args;
		String* output     = nullptr;
		ExecuteFlags flags = ExecuteFlags::Undefined;
	};

	class ENGINE_EXPORT Entry
	{
	protected:
		class ArrayInterface
		{
		public:
			virtual usize size() const                        = 0;
			virtual void clear()                              = 0;
			virtual ExecuteStatus append(const Argument* src) = 0;
		};

		template<EmplaceBackContainer Container>
		class EmplaceBackArray final : public ArrayInterface
		{
		private:
			Container* m_container;

		public:
			EmplaceBackArray(Container* container) : m_container(container) {}

			usize size() const override
			{
				if constexpr (requires { m_container->size(); })
					return static_cast<usize>(m_container->size());
				else
					return static_cast<usize>(m_container->length());
			}

			void clear() override { m_container->clear(); }

			ExecuteStatus append(const Argument* src) override
			{
				auto& value = m_container->emplace_back();

				ExecuteStatus status = Entry::store(&value, src);

				if (status != ExecuteStatus::Success)
				{
					m_container->pop_back();
					return status;
				}

				return ExecuteStatus::Success;
			}
		};

		template<IndexedContainer Container>
		class IndexedArray final : public ArrayInterface
		{
		private:
			Container* m_container;
			usize m_index = 0;

		public:
			IndexedArray(Container* container) : m_container(container) {}

			usize size() const override
			{
				if constexpr (requires { m_container->size(); })
					return static_cast<usize>(m_container->size());
				else
					return static_cast<usize>(m_container->length());
			}

			void clear() override {}

			ExecuteStatus append(const Argument* src) override
			{
				if (m_index >= size())
					return ExecuteStatus::ValueParseFailed;

				ExecuteStatus status = Entry::store(&(*m_container)[m_index++], src);

				if (status != ExecuteStatus::Success)
				{
					return status;
				}

				return ExecuteStatus::Success;
			}
		};

		static ExecuteStatus store(bool* dst, const Argument* src);
		static ExecuteStatus store(u64* dst, const Argument* src);
		static ExecuteStatus store(i64* dst, const Argument* src);
		static ExecuteStatus store(f64* dst, const Argument* src);
		static ExecuteStatus store(String* dst, const Argument* src);
		static ExecuteStatus store(ArrayInterface* dst, const Argument* src);
		static ExecuteStatus store_enum(u64* dst, const Argument* src, Refl::Enum* refl);
		static ExecuteStatus store_bitfield(u64* dst, const Argument* src, Refl::Enum* refl);

		static ExecuteStatus store(String* dst, bool* src);
		static ExecuteStatus store(String* dst, u64* src);
		static ExecuteStatus store(String* dst, i64* src);
		static ExecuteStatus store(String* dst, f64* src);
		static ExecuteStatus store(String* dst, String* src);
		static ExecuteStatus store(String* dst, ArrayInterface* src);

		template<etl::signed_integral T>
		    requires(!etl::same_as<T, bool>)
		static ExecuteStatus store(String* dst, T* src)
		{
			i64 value = static_cast<i64>(*src);
			return store(dst, &value);
		}

		template<etl::unsigned_integral T>
		    requires(!etl::same_as<T, bool>)
		static ExecuteStatus store(String* dst, T* src)
		{
			u64 value = static_cast<u64>(*src);
			return store(dst, &value);
		}

		template<etl::floating_point T>
		static ExecuteStatus store(String* dst, T* src)
		{
			f64 value = static_cast<f64>(*src);
			return store(dst, &value);
		}

		template<typename DstType, typename SrcType>
		static ExecuteStatus store(DstType* dst, const Argument* src)
		{
			SrcType result;
			ExecuteStatus status = store(&result, src);

			if (status != ExecuteStatus::Success)
				return status;

			(*dst) = static_cast<DstType>(result);
			return ExecuteStatus::Success;
		}

		template<etl::signed_integral T>
		    requires(!etl::same_as<T, bool>)
		static ExecuteStatus store(T* dst, const Argument* src)
		{
			return store<T, i64>(dst, src);
		}

		template<etl::unsigned_integral T>
		    requires(!etl::same_as<T, bool>)
		static ExecuteStatus store(T* dst, const Argument* src)
		{
			return store<T, u64>(dst, src);
		}

		template<etl::floating_point T>
		static ExecuteStatus store(T* dst, const Argument* src)
		{
			return store<T, f64>(dst, src);
		}

		template<EmplaceBackContainer Container>
		static ExecuteStatus store(Container* dst, const Argument* src)
		{
			EmplaceBackArray array(dst);
			return store(static_cast<ArrayInterface*>(&array), src);
		}

		template<IndexedContainer Container>
		static ExecuteStatus store(Container* dst, const Argument* src)
		{
			IndexedArray array(dst);
			return store(static_cast<ArrayInterface*>(&array), src);
		}

		template<etl::regular_reflected_enum EnumType>
		static ExecuteStatus store(EnumType* value, const Argument* src)
		{
			u64 tmp              = value->value;
			ExecuteStatus result = store_enum(&tmp, src, EnumType::static_reflection());

			if (result == ExecuteStatus::Success)
				value->value = tmp;

			return result;
		}

		template<etl::bitfield_reflected_enum EnumType>
		static ExecuteStatus store(EnumType* value, const Argument* src)
		{
			u64 tmp              = value->bitfield;
			ExecuteStatus result = store_bitfield(&tmp, src, EnumType::static_reflection());

			if (result == ExecuteStatus::Success)
				value->bitfield = tmp;

			return result;
		}

	private:
		StringView m_name;
		StringView m_description;
		EntryFlags m_flags;

	public:
		Entry(StringView name, StringView description = "", EntryFlags flags = EntryFlags::Undefined);
		virtual ~Entry();

		virtual EntryType type() const                           = 0;
		virtual ExecuteStatus execute(const ExecuteContext& ctx) = 0;

		inline StringView name() const { return m_name; }
		inline StringView description() const { return m_description; }
		inline EntryFlags flags() const { return m_flags; }
	};

	class ENGINE_EXPORT VariableEntry : public Entry
	{
	public:
		using Entry::Entry;

		virtual const void* data() const = 0;
		virtual void* data()             = 0;

		EntryType type() const override;
	};

	template<typename T, typename Scope>
	class TypedVariableBase : public VariableEntry
	{
	public:
		using VariableEntry::VariableEntry;

		ExecuteStatus execute(const ExecuteContext& ctx) override
		{
			if (ctx.args.size() == 0)
			{
				if (ctx.output)
				{
					//return Entry::store(ctx.output, &value());
				}

				return ExecuteStatus::Success;
			}

			if (ctx.args.size() != 1)
			{
				return ExecuteStatus::TooManyParameters;
			}

			return Entry::store(&value(), ctx.args.data());
		}

		inline T& value() { return *static_cast<T*>(static_cast<Scope*>(this)->data()); }
		inline const T& value() const { return *static_cast<T*>(static_cast<Scope*>(this)->data()); }
		inline operator T&() { return value(); }
		inline operator const T&() const { return value(); }
	};

	template<typename T>
	class VariableRef final : public TypedVariableBase<T, VariableRef<T>>
	{
	private:
		T* m_value;

	public:
		VariableRef(T* value, StringView name, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
		    : TypedVariableBase<T, VariableRef<T>>(name, description, flags), m_value(value)
		{}

		const void* data() const override { return m_value; }
		void* data() override { return m_value; }
	};

	template<typename T>
	class Variable final : public TypedVariableBase<T, Variable<T>>
	{
	private:
		T m_value;

	public:
		Variable(StringView name, const T& value = {}, StringView description = "", EntryFlags flags = EntryFlags::Undefined)
		    : TypedVariableBase<T, Variable<T>>(name, description, flags), m_value(value)
		{}

		const void* data() const override { return &m_value; }
		void* data() override { return &m_value; }
	};

	class ENGINE_EXPORT Command : public Entry
	{
	public:
		using Callback = Function<ExecuteStatus(const ExecuteContext&)>;

	private:
		Callback m_callback;

	public:
		Command(StringView name, Callback callback, StringView description = "", EntryFlags flags = EntryFlags::Undefined);

		template<typename... Args>
		Command(StringView name, void (*callback)(Args...), StringView description = "", EntryFlags flags = EntryFlags::Undefined)
		    : Command(name, bind(callback), description, flags)
		{}

		EntryType type() const override;
		ExecuteStatus execute(const ExecuteContext& ctx) override;

	private:
		template<typename... Args>
		static Callback bind(void (*callback)(Args...))
		{
			return [callback](const ExecuteContext& ctx) -> ExecuteStatus {
				if (ctx.args.size() < sizeof...(Args))
				{
					return ExecuteStatus::MissingRequiredParameter;
				}

				if (ctx.args.size() > sizeof...(Args))
				{
					return ExecuteStatus::TooManyParameters;
				}

				return execute(callback, ctx, etl::index_sequence_for<Args...>());
			};
		}

		template<typename... Args, usize... Indices>
		static ExecuteStatus execute(void (*callback)(Args...), const ExecuteContext& ctx, etl::index_sequence<Indices...>)
		{
			Tuple<std::remove_cvref_t<Args>...> args;
			ExecuteStatus status = ExecuteStatus::Success;

			((status == ExecuteStatus::Success ? status = Entry::store(&etl::get<Indices>(args), &ctx.args[Indices]) : status),
			 ...);

			if (status != ExecuteStatus::Success)
			{
				return status;
			}

			callback(etl::get<Indices>(args)...);
			return ExecuteStatus::Success;
		}
	};


	ENGINE_EXPORT Entry* find(StringView name);
	ENGINE_EXPORT usize find(StringView name, const FunctionRef<void(Entry*)>& action);
	ENGINE_EXPORT ExecuteStatus execute(StringView source, String* output = nullptr, ExecuteFlags flags = {});
	ENGINE_EXPORT ExecuteStatus execute_config(const Path& path);

#define trinex_console_variable(type, var, name, ...)                                                                            \
	Trinex::Console::Variable<type> var = Trinex::Console::Variable<type>(name __VA_OPT__(, ) __VA_ARGS__)

#define trinex_console_command(name, ...)                                                                                        \
	static void TRINEX_CONCAT(trinex_console_command_callback_, __LINE__)(__VA_ARGS__);                                          \
	static Trinex::Console::Command TRINEX_CONCAT(trinex_console_command_entry_,                                                 \
	                                              __LINE__)(#name, &TRINEX_CONCAT(trinex_console_command_callback_, __LINE__));  \
	static void TRINEX_CONCAT(trinex_console_command_callback_, __LINE__)(__VA_ARGS__)
}// namespace Trinex::Console
