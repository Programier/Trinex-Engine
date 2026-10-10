#include <cassert>
#include <stream.hpp>

namespace Reflector
{
	CodeWriter::Indentation::Indentation(CodeWriter& writer) : m_writer(writer), m_previous_level(writer.indent_level())
	{
		m_writer.indent();
	}

	CodeWriter::Indentation::~Indentation()
	{
		while (m_writer.indent_level() > m_previous_level) m_writer.unindent();
	}

	CodeWriter::CodeWriter(std::string_view indent) : m_indent(indent) {}

	CodeWriter& CodeWriter::write(std::string_view text)
	{
		for (std::size_t index = 0; index < text.size(); ++index)
		{
			char character = text[index];
			if (character == '\r')
			{
				if (index + 1 < text.size() && text[index + 1] == '\n')
					++index;
				character = '\n';
			}

			if (m_line_start && character != '\n')
			{
				for (std::size_t level = 0; level < m_indent_level; ++level) m_text += m_indent;
				m_line_start = false;
			}

			m_text += character;
			if (character == '\n')
				m_line_start = true;
		}
		return *this;
	}

	CodeWriter& CodeWriter::line(std::string_view text)
	{
		return write(text).write("\n");
	}

	CodeWriter& CodeWriter::indent()
	{
		++m_indent_level;
		return *this;
	}

	CodeWriter& CodeWriter::unindent()
	{
		assert(m_indent_level > 0);
		--m_indent_level;
		return *this;
	}

	std::size_t CodeWriter::indent_level() const noexcept
	{
		return m_indent_level;
	}

	CodeWriter::Indentation CodeWriter::scoped_indent()
	{
		return Indentation(*this);
	}

	std::string_view CodeWriter::text() const noexcept
	{
		return m_text;
	}
}// namespace Reflector
