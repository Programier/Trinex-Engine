#include <ostream>
#include <stream.hpp>

namespace Reflector
{
	namespace
	{
		class CodeStreamBuf final : public std::streambuf
		{
		public:
			explicit CodeStreamBuf(std::streambuf* destination, std::size_t indent_size = 4)
			    : m_destination(destination), m_indent_size(indent_size)
			{}

			void indent() noexcept { ++m_indent; }

			void unindent() noexcept
			{
				if (m_indent > 0)
					--m_indent;
			}

			std::size_t indent_level() const noexcept { return m_indent; }

			void indent_size(std::size_t size) noexcept { m_indent_size = size; }

		protected:
			int_type overflow(int_type ch) override
			{
				if (traits_type::eq_int_type(ch, traits_type::eof()))
					return traits_type::not_eof(ch);

				const char c = traits_type::to_char_type(ch);

				if (m_line_start && c != '\n')
				{
					const std::size_t count = m_indent * m_indent_size;

					for (std::size_t i = 0; i < count; ++i)
					{
						if (traits_type::eq_int_type(m_destination->sputc(' '), traits_type::eof()))
						{
							return traits_type::eof();
						}
					}

					m_line_start = false;
				}

				if (traits_type::eq_int_type(m_destination->sputc(c), traits_type::eof()))
				{
					return traits_type::eof();
				}

				if (c == '\n')
					m_line_start = true;

				return traits_type::to_int_type(c);
			}

			std::streamsize xsputn(const char* data, std::streamsize count) override
			{
				std::streamsize written = 0;

				for (; written < count; ++written)
				{
					if (traits_type::eq_int_type(overflow(traits_type::to_int_type(data[written])), traits_type::eof()))
					{
						break;
					}
				}

				return written;
			}

			int sync() override { return m_destination->pubsync(); }

		private:
			std::streambuf* m_destination = nullptr;

			std::size_t m_indent      = 0;
			std::size_t m_indent_size = 4;

			bool m_line_start = true;
		};

		class CodeStream final : public std::ostream
		{
		public:
			explicit CodeStream(std::ostream& destination, std::size_t indent_size = 4)
			    : std::ostream(nullptr), m_destination(destination), m_buffer(destination.rdbuf(), indent_size)
			{
				this->init(&m_buffer);

				// Preserve formatting configuration.
				this->copyfmt(destination);
			}

			CodeStream(const CodeStream&)            = delete;
			CodeStream& operator=(const CodeStream&) = delete;

			~CodeStream() override { flush(); }

			void indent() noexcept { m_buffer.indent(); }

			void unindent() noexcept { m_buffer.unindent(); }

			std::size_t indent_level() const noexcept { return m_buffer.indent_level(); }

			void indent_size(std::size_t size) noexcept { m_buffer.indent_size(size); }

			std::ostream& destination() noexcept { return m_destination; }

		private:
			std::ostream& m_destination;
			CodeStreamBuf m_buffer;
		};
	}// namespace

	std::unique_ptr<std::ostream> code_stream(std::ostream& stream)
	{
		return std::make_unique<CodeStream>(stream);
	}

	std::ostream& indent(std::ostream& stream)
	{
		if (CodeStream* code = dynamic_cast<CodeStream*>(&stream))
		{
			code->indent();
		}

		return stream;
	}

	std::ostream& unindent(std::ostream& stream)
	{
		if (CodeStream* code = dynamic_cast<CodeStream*>(&stream))
		{
			code->unindent();
		}
		return stream;
	}
}// namespace Reflector
