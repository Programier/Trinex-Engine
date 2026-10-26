#pragma once
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Reflector
{
	class Object;
	class CodeWriter;

	template<typename Executor>
	concept CodeWriterExecutor = requires(CodeWriter* writer, const Object* object, Executor executor) {
		{ executor.execute(writer, object) };
	};

	template<typename Executor>
	struct WriteAs
	{
		static_assert(CodeWriterExecutor<Executor>,
		              "WriteAs executor must provide static void execute(CodeWriter*, const Object*)");

		const Object* object;
	};

	template<typename T>
	inline constexpr bool is_write_as = false;

	template<typename Executor>
	inline constexpr bool is_write_as<WriteAs<Executor>> = true;

	template<typename T>
	concept CodeWriterStringifiable = requires(T&& value) {
		{ std::to_string(std::forward<T>(value)) } -> std::convertible_to<std::string>;
	};

	template<typename T>
	concept CodeWriterLine = std::convertible_to<T, std::string_view> || std::convertible_to<T, const Object*>
	                         || is_write_as<std::remove_cvref_t<T>> || CodeWriterStringifiable<T>;

	template<typename... Args>
	class CodeWriterFormat final
	{
	private:
		std::string_view m_text;

		template<std::size_t Size>
		static consteval void validate(const char (&text)[Size])
		{
			for (std::size_t index = 0; index + 1 < Size; ++index)
			{
				if (text[index] != '$')
					continue;

				++index;
				if (text[index] == '$')
					continue;

				if (text[index] < '0' || text[index] > '9')
					throw "CodeWriter format placeholders must use $N";

				std::size_t argument = 0;
				do
				{
					argument = argument * 10 + static_cast<std::size_t>(text[index] - '0');
					++index;
				} while (index + 1 < Size && text[index] >= '0' && text[index] <= '9');

				if (argument >= sizeof...(Args))
					throw "CodeWriter format placeholder index is out of range";
				--index;
			}
		}

	public:
		template<std::size_t Size>
		consteval CodeWriterFormat(const char (&text)[Size]) : m_text(text, Size - 1)
		{
			validate(text);
		}

		constexpr std::string_view text() const { return m_text; }
	};

	// An owned text buffer, independent of streams and files. No RTTI or implicit flushing.
	// Implementation is deferred; this header defines the code formatting contract.
	class CodeWriter final
	{
	public:
		// Restores the previous indentation level when leaving the current C++ scope.
		class Indentation final
		{
		private:
			CodeWriter& m_writer;
			std::size_t m_previous_level;

		public:
			explicit Indentation(CodeWriter& writer);
			~Indentation();

			Indentation(const Indentation&)            = delete;
			Indentation& operator=(const Indentation&) = delete;
			Indentation(Indentation&&)                 = delete;
			Indentation& operator=(Indentation&&)      = delete;
		};

	private:
		std::string m_text;
		std::string m_indent;
		std::size_t m_indent_level = 0;
		bool m_line_start          = true;

		template<std::size_t Index = 0, typename Tuple>
		bool write_format_argument(std::size_t index, Tuple&& args)
		{
			if constexpr (Index == std::tuple_size_v<std::remove_reference_t<Tuple>>)
			{
				return false;
			}
			else
			{
				if (index == Index)
				{
					write(std::get<Index>(std::forward<Tuple>(args)));
					return true;
				}

				return write_format_argument<Index + 1>(index, std::forward<Tuple>(args));
			}
		}

	public:
		explicit CodeWriter(std::string_view indent = "\t");

		CodeWriter(const CodeWriter&)            = delete;
		CodeWriter& operator=(const CodeWriter&) = delete;
		CodeWriter(CodeWriter&&)                 = delete;
		CodeWriter& operator=(CodeWriter&&)      = delete;

		CodeWriter& reserve(std::size_t size);

		CodeWriter& write(std::string_view text);
		CodeWriter& line(std::string_view text = {});

		CodeWriter& write(const Object* object);
		CodeWriter& line(const Object* object);

		template<CodeWriterStringifiable T>
		CodeWriter& write(T&& value)
		{
			return write(std::to_string(std::forward<T>(value)));
		}

		template<CodeWriterStringifiable T>
		CodeWriter& line(T&& value)
		{
			return write(std::forward<T>(value)).line();
		}

		template<typename Executor>
		CodeWriter& write(WriteAs<Executor> object)
		{
			Executor instance;
			instance.execute(this, object.object);
			return *this;
		}

		template<typename Executor>
		CodeWriter& line(WriteAs<Executor> object)
		{
			return write(object).write("\n");
		}

		// Replaces $0, $1, ... with the corresponding argument. $$ emits a literal '$'.
		template<CodeWriterLine... Args>
		CodeWriter& write(CodeWriterFormat<std::type_identity_t<Args>...> format, Args&&... args)
		{
			const std::string_view text = format.text();
			auto values                 = std::forward_as_tuple(std::forward<Args>(args)...);

			for (std::size_t index = 0; index < text.size();)
			{
				const std::size_t marker = text.find('$', index);
				if (marker == std::string_view::npos)
				{
					write(text.substr(index));
					break;
				}

				write(text.substr(index, marker - index));
				index = marker + 1;
				if (text[index] == '$')
				{
					write("$");
					++index;
					continue;
				}

				std::size_t argument = 0;
				do
				{
					argument = argument * 10 + static_cast<std::size_t>(text[index] - '0');
					++index;
				} while (index < text.size() && text[index] >= '0' && text[index] <= '9');

				write_format_argument(argument, values);
			}

			return *this;
		}

		template<CodeWriterLine... Args>
		CodeWriter& line(CodeWriterFormat<std::type_identity_t<Args>...> format, Args&&... args)
		{
			return write(format, std::forward<Args>(args)...).write("\n");
		}

		CodeWriter& indent();
		CodeWriter& unindent();
		[[nodiscard]] Indentation scope();

		// The returned view remains valid until the next mutation or destruction.
		std::string_view text() const noexcept;
	};
} // namespace Reflector
