#pragma once
#include <cstddef>
#include <string>
#include <string_view>

namespace Reflector
{
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

		CodeWriter& write(std::string_view text);
		CodeWriter& line(std::string_view text = {});

		CodeWriter& indent();
		// Unindenting at level zero is a programming error, not a silent no-op.
		CodeWriter& unindent();
		std::size_t indent_level() const noexcept;
		[[nodiscard]] Indentation scoped_indent();

		// The returned view remains valid until the next mutation or destruction.
		std::string_view text() const noexcept;
	};
}// namespace Reflector
