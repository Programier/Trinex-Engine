#pragma once
#include <Core/etl/any.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <type_traits>

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

	struct ValueType {
		enum Enum : u8
		{
			Boolean        = 0,
			U8             = 1,
			U16            = 2,
			U32            = 3,
			U64            = 4,
			I8             = 5,
			I16            = 6,
			I32            = 7,
			I64            = 8,
			F16            = 9,
			F32            = 10,
			F64            = 11,
			String         = 12,
			ReflectedEnum  = 13,
			ReflectedFlags = 14,
		};

		trinex_enum_struct(ValueType);
	};

	struct Flags {
		enum Enum : u32
		{
			Undefined     = 0,
			Persistent    = 1 << 0,
			ReadOnly      = 1 << 1,
			Hidden        = 1 << 2,
			Cheat         = 1 << 3,
			DeveloperOnly = 1 << 4,
		};

		trinex_bitfield_enum_struct(Flags, u32);
	};

	struct ExecuteFlags {
		enum Enum : u32
		{
			Undefined        = 0,
			IgnoreUnknown    = 1 << 0,
			StrictParameters = 1 << 1,
			SingleCommand    = 1 << 2,
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
			DeveloperOnly,
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

	struct ExecuteResult {
		String output;
		ExecuteStatus status = ExecuteStatus::Success;

		bool success() const { return status == ExecuteStatus::Success; }
		operator const String&() const { return output; }
	};

	struct RuntimePolicy {
		bool allow_cheats         = false;
		bool allow_developer_only = true;
		bool show_hidden          = false;
	};

	class Entry;
	class Command;
	class VariableEntry;

	namespace Detail
	{
		template<typename Type>
		inline constexpr bool always_false_v = false;

		template<typename Type>
		struct VariableTypeTag {
			static_assert(always_false_v<Type>, "Console variable type is not supported");
		};

		template<>
		struct VariableTypeTag<bool> {
			static constexpr ValueType value = ValueType::Boolean;
		};

		template<>
		struct VariableTypeTag<u8> {
			static constexpr ValueType value = ValueType::U8;
		};

		template<>
		struct VariableTypeTag<u16> {
			static constexpr ValueType value = ValueType::U16;
		};

		template<>
		struct VariableTypeTag<u32> {
			static constexpr ValueType value = ValueType::U32;
		};

		template<>
		struct VariableTypeTag<u64> {
			static constexpr ValueType value = ValueType::U64;
		};

		template<>
		struct VariableTypeTag<i8> {
			static constexpr ValueType value = ValueType::I8;
		};

		template<>
		struct VariableTypeTag<i16> {
			static constexpr ValueType value = ValueType::I16;
		};

		template<>
		struct VariableTypeTag<i32> {
			static constexpr ValueType value = ValueType::I32;
		};

		template<>
		struct VariableTypeTag<i64> {
			static constexpr ValueType value = ValueType::I64;
		};

		template<>
		struct VariableTypeTag<f16> {
			static constexpr ValueType value = ValueType::F16;
		};

		template<>
		struct VariableTypeTag<f32> {
			static constexpr ValueType value = ValueType::F32;
		};

		template<>
		struct VariableTypeTag<f64> {
			static constexpr ValueType value = ValueType::F64;
		};

		template<>
		struct VariableTypeTag<String> {
			static constexpr ValueType value = ValueType::String;
		};

		template<typename Type>
		    requires(requires {
			    Type::is_enum;
			    Type::is_bitfield_enum;
			    typename Type::Enum;
		    } && Type::is_enum && !Type::is_bitfield_enum)
		struct VariableTypeTag<Type> {
			static constexpr ValueType value = ValueType::ReflectedEnum;
		};

		template<typename Type>
		    requires(requires {
			    Type::is_enum;
			    Type::is_bitfield_enum;
			    typename Type::Enum;
		    } && Type::is_enum && Type::is_bitfield_enum)
		struct VariableTypeTag<Type> {
			static constexpr ValueType value = ValueType::ReflectedFlags;
		};

		ENGINE_EXPORT bool parse_boolean(StringView raw_value, bool& out_value);
		ENGINE_EXPORT bool parse_signed(StringView raw_value, i64& out_value);
		ENGINE_EXPORT bool parse_unsigned(StringView raw_value, u64& out_value);
		ENGINE_EXPORT bool parse_floating(StringView raw_value, f64& out_value);
		ENGINE_EXPORT bool parse_string(StringView raw_value, String& out_value);
		ENGINE_EXPORT bool parse_reflected_enum(StringView raw_value, Refl::Enum* reflection, bool is_bitfield, u64& out_value);

		ENGINE_EXPORT String format_boolean(bool value);
		ENGINE_EXPORT String format_signed(i64 value);
		ENGINE_EXPORT String format_unsigned(u64 value);
		ENGINE_EXPORT String format_floating(f64 value);
		ENGINE_EXPORT String format_string(StringView value);
		ENGINE_EXPORT String format_reflected_enum(Refl::Enum* reflection, u64 value, bool is_bitfield);
		ENGINE_EXPORT String format_parse_error(StringView name, StringView raw_value);
		ENGINE_EXPORT String format_assignment(StringView name, StringView value);

		template<typename Type>
		static bool parse_value(StringView raw_value, Type& out_value)
		{
			if constexpr (std::is_same_v<Type, bool>)
			{
				return parse_boolean(raw_value, out_value);
			}
			else if constexpr (std::is_same_v<Type, String>)
			{
				return parse_string(raw_value, out_value);
			}
			else if constexpr (std::is_same_v<Type, i8> || std::is_same_v<Type, i16> || std::is_same_v<Type, i32> ||
			                   std::is_same_v<Type, i64>)
			{
				i64 value = 0;

				if (!parse_signed(raw_value, value))
					return false;

				out_value = static_cast<Type>(value);
				return true;
			}
			else if constexpr (std::is_same_v<Type, u8> || std::is_same_v<Type, u16> || std::is_same_v<Type, u32> ||
			                   std::is_same_v<Type, u64>)
			{
				u64 value = 0;

				if (!parse_unsigned(raw_value, value))
					return false;

				out_value = static_cast<Type>(value);
				return true;
			}
			else if constexpr (std::is_same_v<Type, f16> || std::is_same_v<Type, f32> || std::is_same_v<Type, f64>)
			{
				f64 value = 0.0;

				if (!parse_floating(raw_value, value))
					return false;

				out_value = static_cast<Type>(value);
				return true;
			}
			else if constexpr (requires {
				                   Type::is_enum;
				                   Type::is_bitfield_enum;
				                   typename Type::Enum;
			                   } && Type::is_enum)
			{
				using Underlying = std::underlying_type_t<typename Type::Enum>;

				if constexpr (requires { Type::is_enum_reflected; } && Type::is_enum_reflected)
				{
					u64 value = 0;
					if (parse_reflected_enum(raw_value, Type::static_reflection(), Type::is_bitfield_enum, value))
					{
						if constexpr (Type::is_bitfield_enum)
							out_value = Type(static_cast<Underlying>(value));
						else
							out_value = Type(static_cast<typename Type::Enum>(value));
						return true;
					}
				}

				if constexpr (std::is_signed_v<Underlying>)
				{
					i64 value = 0;
					if (!parse_signed(raw_value, value))
						return false;

					if constexpr (Type::is_bitfield_enum)
						out_value = Type(static_cast<Underlying>(value));
					else
						out_value = Type(static_cast<typename Type::Enum>(value));
				}
				else
				{
					u64 value = 0;
					if (!parse_unsigned(raw_value, value))
						return false;

					if constexpr (Type::is_bitfield_enum)
						out_value = Type(static_cast<Underlying>(value));
					else
						out_value = Type(static_cast<typename Type::Enum>(value));
				}

				return true;
			}
			else
			{
				static_assert(always_false_v<Type>, "Console variable type is not supported");
			}
		}

		template<typename Type>
		static bool parse_any(StringView raw_value, Any& out_value)
		{
			Type value{};

			if (!parse_value(raw_value, value))
				return false;

			out_value = std::move(value);
			return true;
		}

		template<typename Type>
		static String format_type_name()
		{
			if constexpr (std::is_same_v<Type, bool>)
				return "bool";
			else if constexpr (std::is_same_v<Type, u8>)
				return "u8";
			else if constexpr (std::is_same_v<Type, u16>)
				return "u16";
			else if constexpr (std::is_same_v<Type, u32>)
				return "u32";
			else if constexpr (std::is_same_v<Type, u64>)
				return "u64";
			else if constexpr (std::is_same_v<Type, i8>)
				return "i8";
			else if constexpr (std::is_same_v<Type, i16>)
				return "i16";
			else if constexpr (std::is_same_v<Type, i32>)
				return "i32";
			else if constexpr (std::is_same_v<Type, i64>)
				return "i64";
			else if constexpr (std::is_same_v<Type, f16>)
				return "f16";
			else if constexpr (std::is_same_v<Type, f32>)
				return "f32";
			else if constexpr (std::is_same_v<Type, f64>)
				return "f64";
			else if constexpr (std::is_same_v<Type, String>)
				return "String";
			else
				return String(type_info<Type>::name());
		}

		template<typename Type>
		static String format_value(const Type& value)
		{
			if constexpr (std::is_same_v<Type, bool>)
			{
				return format_boolean(value);
			}
			else if constexpr (std::is_same_v<Type, String>)
			{
				return format_string(value);
			}
			else if constexpr (std::is_same_v<Type, i8> || std::is_same_v<Type, i16> || std::is_same_v<Type, i32> ||
			                   std::is_same_v<Type, i64>)
			{
				return format_signed(static_cast<i64>(value));
			}
			else if constexpr (std::is_same_v<Type, u8> || std::is_same_v<Type, u16> || std::is_same_v<Type, u32> ||
			                   std::is_same_v<Type, u64>)
			{
				return format_unsigned(static_cast<u64>(value));
			}
			else if constexpr (std::is_same_v<Type, f16> || std::is_same_v<Type, f32> || std::is_same_v<Type, f64>)
			{
				return format_floating(static_cast<f64>(value));
			}
			else if constexpr (requires {
				                   Type::is_enum;
				                   Type::is_bitfield_enum;
				                   typename Type::Enum;
			                   } && Type::is_enum)
			{
				using Underlying = std::underlying_type_t<typename Type::Enum>;

				if constexpr (requires { Type::is_enum_reflected; } && Type::is_enum_reflected)
				{
					String reflected = format_reflected_enum(
					        Type::static_reflection(), static_cast<u64>(static_cast<Underlying>(value)), Type::is_bitfield_enum);
					if (!reflected.empty())
						return reflected;
				}

				if constexpr (std::is_signed_v<Underlying>)
					return format_signed(static_cast<i64>(static_cast<Underlying>(value)));
				else
					return format_unsigned(static_cast<u64>(static_cast<Underlying>(value)));
			}
			else
			{
				static_assert(always_false_v<Type>, "Console variable type is not supported");
			}
		}
	}// namespace Detail

	struct VariableChangedEvent {
		const VariableEntry& variable;
		StringView input;
	};

	struct CommandParameter {
		String name;
		String type;
		String description;
		bool (*parse)(StringView raw_value, Any& out_value) = nullptr;
		Function<String(const Any&)> validate               = nullptr;
		Any default_value;
		String default_value_text;
		bool has_default_value = false;
	};

	struct CommandExecutedEvent {
		StringView input;
		StringView command;
		String output;
		ExecuteStatus status = ExecuteStatus::Success;

		bool success() const { return status == ExecuteStatus::Success; }
	};

	struct ConfigLoadedEvent {
		String path;
		String output;
		ExecuteStatus status = ExecuteStatus::Success;
		bool persistent_only;

		bool success() const { return status == ExecuteStatus::Success; }
	};

	class ENGINE_EXPORT Observer
	{
	public:
		virtual ~Observer() = default;
		virtual void on_variable_changed(const VariableChangedEvent&) {}
		virtual void on_command_executed(const CommandExecutedEvent&) {}
		virtual void on_config_loaded(const ConfigLoadedEvent&) {}
	};

	struct ENGINE_EXPORT StackFrame {
		Entry* entry = nullptr;
		StringView stream;
		StringView input;
		String result;
		String error;
		ExecuteStatus status = ExecuteStatus::Success;
		ExecuteFlags flags   = ExecuteFlags::Undefined;
		Vector<Any> parameters;

		StackFrame(Entry* entry = nullptr, StringView stream = {}, StringView input = {},
		           ExecuteFlags flags = ExecuteFlags::Undefined);
		~StackFrame();

		void reset(Entry* entry = nullptr, StringView stream = {}, StringView input = {},
		           ExecuteFlags flags = ExecuteFlags::Undefined);
		bool read_argument(StringView& out);
		StringView read_command_tail();
		bool has_more_args();
		bool failed() const { return status != ExecuteStatus::Success || !error.empty(); }
		String fail(ExecuteStatus status, StringView message);
		String succeed(StringView message = {});

		template<typename Type>
		Type& get(usize index)
		{
			return *parameters[index].address_as<Type>();
		}

		template<typename Type>
		const Type& get(usize index) const
		{
			return *parameters[index].address_as<Type>();
		}

		template<typename Type>
		Type& get(StringView name);

		template<typename Type>
		const Type& get(StringView name) const;

		template<typename Type>
		bool parse_arg(Type& out_value)
		{
			StringView argument;

			if (!read_argument(argument))
				return false;

			return Detail::parse_value(argument, out_value);
		}

		template<typename Type>
		bool require_arg(Type& out_value, StringView label = "argument")
		{
			if (!has_more_args())
			{
				status = ExecuteStatus::MissingRequiredParameter;
				error  = String("Expected ") + String(label);
				return false;
			}

			if (!parse_arg(out_value))
			{
				status = ExecuteStatus::ParameterParseFailed;
				error  = String("Expected ") + String(label);
				return false;
			}

			return true;
		}

		template<typename Type>
		Type optional_arg(const Type& default_value)
		{
			Type value = default_value;

			if (has_more_args())
			{
				if (!parse_arg(value))
				{
					status = ExecuteStatus::ParameterParseFailed;
					error  = "Failed to parse optional argument";
				}
			}

			return value;
		}
	};

	ENGINE_EXPORT StringView type_name(ValueType type);
	ENGINE_EXPORT Entry* find(StringView name);
	ENGINE_EXPORT bool exists(StringView name);
	ENGINE_EXPORT Vector<String> entries();
	ENGINE_EXPORT Vector<String> complete(StringView prefix);
	ENGINE_EXPORT ExecuteResult execute_view(StringView& stream, ExecuteFlags flags = ExecuteFlags::Undefined);
	ENGINE_EXPORT ExecuteResult execute(StringView stream, ExecuteFlags flags = ExecuteFlags::Undefined);
	ENGINE_EXPORT ExecuteResult execute(int argc, char** argv, ExecuteFlags flags = ExecuteFlags::Undefined);
	ENGINE_EXPORT RuntimePolicy runtime_policy();
	ENGINE_EXPORT void runtime_policy(const RuntimePolicy& policy);
	ENGINE_EXPORT void add_observer(Observer* observer);
	ENGINE_EXPORT void remove_observer(Observer* observer);
	ENGINE_EXPORT void add_alias(StringView name, StringView expansion);
	ENGINE_EXPORT bool remove_alias(StringView name);
	ENGINE_EXPORT String find_alias(StringView name);
	ENGINE_EXPORT Vector<String> aliases();

	class ENGINE_EXPORT Entry
	{
	private:
		String m_name;
		String m_description;
		String m_category;

	protected:
		Entry(StringView name, StringView description, StringView category);

		static inline StringView resolve_name(StringView name, StringView override_name)
		{
			return override_name.empty() ? name : override_name;
		}

	public:
		virtual ~Entry();

		inline const String& name() const { return m_name; }
		inline const String& description() const { return m_description; }
		inline const String& category() const { return m_category; }
		Entry& name(StringView value);
		Entry& description(StringView value);
		Entry& category(StringView value);

		virtual EntryType type() const            = 0;
		virtual String execute(StackFrame* frame) = 0;
	};

	template<typename Type>
	class CommandParameterBuilder
	{
	private:
		CommandParameter m_parameter;

	public:
		explicit CommandParameterBuilder(StringView name)
		{
			m_parameter.name  = String(name);
			m_parameter.type  = Detail::format_type_name<Type>();
			m_parameter.parse = &Detail::parse_any<Type>;
		}

		CommandParameterBuilder& description(StringView value)
		{
			m_parameter.description = String(value);
			return *this;
		}

		CommandParameterBuilder& default_value(const Type& value)
		{
			m_parameter.default_value      = value;
			m_parameter.default_value_text = Detail::format_value(value);
			m_parameter.has_default_value  = true;
			return *this;
		}

		template<typename Callback>
		CommandParameterBuilder& validate(Callback&& callback)
		{
			if constexpr (std::is_invocable_r_v<String, Callback&, const Type&>)
			{
				auto wrapper         = std::forward<Callback>(callback);
				m_parameter.validate = [wrapper](const Any& value) mutable -> String { return wrapper(value.cast<Type>()); };
			}
			else if constexpr (std::is_invocable_r_v<bool, Callback&, const Type&>)
			{
				auto wrapper         = std::forward<Callback>(callback);
				m_parameter.validate = [wrapper](const Any& value) mutable -> String {
					return wrapper(value.cast<Type>()) ? String() : String("Validation failed");
				};
			}
			else
			{
				static_assert(Detail::always_false_v<Callback>, "Command parameter validator signature is not supported");
			}

			return *this;
		}

		operator CommandParameter() const { return m_parameter; }
	};

	template<typename Type>
	CommandParameterBuilder<Type> parameter(StringView name)
	{
		return CommandParameterBuilder<Type>(name);
	}

	template<typename Type>
	class Variable;

	template<typename Type>
	struct VariableChangeContext {
		Variable<Type>& variable;
		const Type& old_value;
		const Type& new_value;
		StringView input;
	};

	template<typename Type>
	class VariableOptions
	{
	private:
		Variable<Type>* m_variable                               = nullptr;
		Function<void(VariableChangeContext<Type>&)> m_on_change = nullptr;
		Function<String(const Type&, StringView)> m_validator    = nullptr;
		Flags m_flags                                            = Flags::Undefined;

	public:
		VariableOptions() = default;
		explicit VariableOptions(Variable<Type>* variable) : m_variable(variable) {}

		VariableOptions& name(StringView value)
		{
			m_variable->Entry::name(value);
			return *this;
		}

		VariableOptions& description(StringView value)
		{
			m_variable->Entry::description(value);
			return *this;
		}

		VariableOptions& category(StringView value)
		{
			m_variable->Entry::category(value);
			return *this;
		}

		template<typename Callback>
		VariableOptions& on_change(Callback&& callback)
		{
			m_on_change = std::forward<Callback>(callback);
			return *this;
		}

		template<typename Callback>
		VariableOptions& validate(Callback&& callback)
		{
			if constexpr (std::is_invocable_r_v<String, Callback&, const Type&, StringView>)
			{
				m_validator = std::forward<Callback>(callback);
			}
			else if constexpr (std::is_invocable_r_v<bool, Callback&, const Type&, StringView>)
			{
				auto wrapper = std::forward<Callback>(callback);
				m_validator  = [wrapper](const Type& value, StringView input) mutable -> String {
                    return wrapper(value, input) ? String() : String("Validation failed");
				};
			}
			else
			{
				static_assert(Detail::always_false_v<Callback>, "Console validator signature is not supported");
			}

			return *this;
		}

		VariableOptions& flags(Flags flags)
		{
			m_flags = flags;
			m_variable->flags(m_flags);
			return *this;
		}

		VariableOptions& add_flags(Flags flags)
		{
			m_flags |= flags;
			m_variable->flags(m_flags);
			return *this;
		}

		VariableOptions& persistent(bool enabled = true)
		{
			if (enabled)
				m_flags.set(Flags::Persistent);
			else
				m_flags.remove(Flags::Persistent);

			m_variable->flags(m_flags);
			return *this;
		}

		Flags flags() const { return m_flags; }

		String validate_value(const Type& value, StringView input) const
		{
			if (m_validator)
				return m_validator(value, input);
			return {};
		}

		void invoke(VariableChangeContext<Type>& context) const
		{
			if (m_on_change)
				m_on_change(context);
		}
	};

	class ENGINE_EXPORT VariableEntry : public Entry
	{
	private:
		ValueType m_value_type;
		Flags m_flags = Flags::Undefined;

	protected:
		VariableEntry(StringView name, StringView description, StringView category, ValueType value_type, Flags flags);

	public:
		EntryType type() const override;
		inline ValueType value_type() const { return m_value_type; }
		inline Flags flags() const { return m_flags; }
		inline VariableEntry& flags(Flags value) { trinex_this_return(m_flags = value); }

		virtual String value_to_string() const                                = 0;
		virtual String default_value_to_string() const                        = 0;
		virtual bool is_default_value() const                                 = 0;
		virtual void reset(StringView input = {})                             = 0;
		virtual String validate(StringView raw_value, StringView input) const = 0;
	};


	template<typename Type>
	struct VariableOptionalArgs {
		Type value = {};
		StringView name;
		StringView description;
		StringView category;
		Flags flags;

		Function<void(VariableChangeContext<Type>&)> on_change;
		Function<String(const Type&, StringView)> validator;
	};

	template<typename Type>
	class Variable final : public VariableEntry
	{
	public:
		static constexpr inline ValueType value_type() { return Detail::VariableTypeTag<Type>::value; }

		struct Builder {
			StringView name;
			Variable operator=(const VariableOptionalArgs<Type>& args) const { return Variable<Type>(name, args); }
		};

	private:
		Type m_value;
		Type m_default_value;

		Function<void(VariableChangeContext<Type>&)> m_on_change;
		Function<String(const Type&, StringView)> m_validator;

		inline String validate_value(const Type& value, StringView input) const
		{
			if (m_validator)
				return m_validator(value, input);
			return {};
		}

	public:
		inline Variable(StringView name, const VariableOptionalArgs<Type>& args = {})
		    : VariableEntry(resolve_name(name, args.name), args.description, args.category, value_type(), args.flags),
		      m_value(args.value), m_default_value(args.value), m_on_change(args.on_change), m_validator(args.validator)
		{}

		const Type& value() const { return m_value; }
		const Type& default_value() const { return m_default_value; }
		Type& value() { return m_value; }

		Variable& set(const Type& value, StringView input = {})
		{
			Type old_value = m_value;
			m_value        = value;

			if (m_on_change)
			{
				VariableChangeContext<Type> context{*this, old_value, m_value, input};
				m_on_change(context);
			}
			return *this;
		}

		Variable& operator=(const Type& value) { return set(value); }
		operator const Type&() const { return m_value; }

		String value_to_string() const override { return Detail::format_value(m_value); }

		String default_value_to_string() const override { return Detail::format_value(m_default_value); }

		bool is_default_value() const override { return m_value == m_default_value; }

		void reset(StringView input = {}) override { set(m_default_value, input); }

		String validate(StringView raw, StringView input) const override
		{
			Type value = {};

			if (!Detail::parse_value(raw, value))
				return Detail::format_parse_error(name(), raw);

			return validate_value(value, input);
		}

		String execute(StackFrame* frame) override
		{
			frame->result.clear();
			frame->error.clear();
			frame->status = ExecuteStatus::Success;

			StringView raw_value;

			if (!frame->read_argument(raw_value))
			{
				return frame->succeed(value_to_string());
			}

			Type value = {};

			if (!Detail::parse_value(raw_value, value))
			{
				return frame->fail(ExecuteStatus::ValueParseFailed, Detail::format_parse_error(name(), raw_value));
			}

			if (String validation = validate_value(value, frame->input); !validation.empty())
			{
				return frame->fail(ExecuteStatus::ValueValidationFailed, validation);
			}

			set(value, frame->input);
			return frame->succeed(Detail::format_assignment(name(), value_to_string()));
		}
	};

	struct CommandOptionalArgs {
		Vector<CommandParameter> parameters;
		StringView name;
		StringView description;
		StringView category;
		StringView usage;
		StringView example;
		Flags flags;
	};

	class ENGINE_EXPORT Command : public Entry
	{
	private:
		String m_usage;
		String m_example;
		Flags m_flags;
		Vector<CommandParameter> m_parameters;

		String (*m_callback)(StackFrame*);

	public:
		explicit Command(StringView name, String (*callback)(StackFrame* frame), const CommandOptionalArgs& args = {});

		EntryType type() const override;
		String execute(StackFrame* frame) override;

		inline const String& usage() const { return m_usage; }
		inline const String& example() const { return m_example; }
		inline Flags flags() const { return m_flags; }
		inline const Vector<CommandParameter>& parameters() const { return m_parameters; }
		inline bool can_invoke_without_parentheses() const
		{
			for (const auto& parameter : m_parameters)
			{
				if (!parameter.has_default_value)
					return false;
			}

			return true;
		}

		inline Command& flags(u32 value) { trinex_this_return(m_flags = value); }
		inline Command& usage(StringView value) { trinex_this_return(m_usage = String(value)); }
		inline Command& example(StringView value) { trinex_this_return(m_example = String(value)); }
		inline String operator()(StackFrame* frame) { return execute(frame); }
	};

	template<typename Type>
	inline Type& StackFrame::get(StringView name)
	{
		return const_cast<Type&>(static_cast<const StackFrame*>(this)->get<Type>(name));
	}

	template<typename Type>
	inline const Type& StackFrame::get(StringView name) const
	{
		trinex_assert(entry != nullptr && entry->type() == EntryType::Command);

		const auto& command    = *static_cast<const Command*>(entry);
		const auto& parameters = command.parameters();

		for (usize index = 0; index < parameters.size(); ++index)
		{
			if (parameters[index].name == name)
				return get<Type>(index);
		}

		trinex_unreachable();
	}
}// namespace Trinex::Console

#define trinex_console_variable(variable_type, variable_name)                                                                    \
	::Trinex::Console::Variable<variable_type> variable_name =                                                                   \
	        ::Trinex::Console::Variable<variable_type>::Builder({.name = #variable_name}) =                                      \
	                ::Trinex::Console::VariableOptionalArgs<variable_type>

#define trinex_extern_console_variable(Type, Name) extern ::Trinex::Console::Variable<Type> Name

#define trinex_console_command(command_name, ...)                                                                                \
	static ::Trinex::String TRINEX_CONCAT(trinex_console_command_, __LINE__)(::Trinex::Console::StackFrame * frame);             \
	::Trinex::Console::Command command_name(#command_name, TRINEX_CONCAT(trinex_console_command_, __LINE__), {__VA_ARGS__});     \
	static ::Trinex::String TRINEX_CONCAT(trinex_console_command_, __LINE__)(::Trinex::Console::StackFrame * frame)

#define trinex_static_console_command(command_name, ...)                                                                         \
	static ::Trinex::String TRINEX_CONCAT(trinex_console_command_, __LINE__)(::Trinex::Console::StackFrame * frame);             \
	static ::Trinex::Console::Command command_name(#command_name, TRINEX_CONCAT(trinex_console_command_, __LINE__),              \
	                                               {__VA_ARGS__});                                                               \
	static ::Trinex::String TRINEX_CONCAT(trinex_console_command_, __LINE__)(::Trinex::Console::StackFrame * frame)

#define trinex_extern_console_command(command_name) extern ::Trinex::Console::Command command_name
