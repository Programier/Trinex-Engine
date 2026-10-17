#pragma once
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

namespace Reflector
{
	class Object;
	class CodeWriter;

	template<typename Executor>
	concept CodeWriterExecutor = requires(CodeWriter* writer, const Object* object, Executor executor) {
		{ executor.execute(writer, object) };
	};

	template<typename Executor>
	struct WriteAs {
		static_assert(CodeWriterExecutor<Executor>,
		              "WriteAs executor must provide static void execute(CodeWriter*, const Object*)");

		const Object* object;
	};

	template<typename T>
	inline constexpr bool is_write_as = false;

	template<typename Executor>
	inline constexpr bool is_write_as<WriteAs<Executor>> = true;

	template<typename T>
	concept CodeWriterLine = std::convertible_to<T, std::string_view> || std::convertible_to<T, const Object*> ||
	                         is_write_as<std::remove_cvref_t<T>>;

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

		template<CodeWriterLine... Args>
		    requires(sizeof...(Args) > 1)
		CodeWriter& write(Args... args)
		{
			return (write(args), ...);
		}

		template<CodeWriterLine... Args>
		    requires(sizeof...(Args) > 1)
		CodeWriter& line(Args... args)
		{
			return (write(args), ...).write("\n");
		}

		CodeWriter& indent();
		// Unindenting at level zero is a programming error, not a silent no-op.
		CodeWriter& unindent();
		std::size_t indent_level() const noexcept;
		[[nodiscard]] Indentation scoped_indent();

		// The returned view remains valid until the next mutation or destruction.
		std::string_view text() const noexcept;
	};
}// namespace Reflector
