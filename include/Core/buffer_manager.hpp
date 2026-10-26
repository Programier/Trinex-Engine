#pragma once
#include <Core/enums.hpp>
#include <Core/etl/array.hpp>
#include <ostream>

namespace Trinex
{
	class trinex_api BufferWriter
	{
	public:
		using Stream    = std::ostream;
		using WritePos  = usize;
		using PosOffset = i64;

		usize size();
		BufferWriter& position(WritePos pos);

		virtual bool write(const u8* data, usize size)                                   = 0;
		virtual WritePos position()                                                      = 0;
		virtual BufferWriter& offset(PosOffset offset, IOWhence dir = IOWhence::Current) = 0;
		virtual bool is_open() const                                                     = 0;

		template<typename... T>
		inline bool write_primitives(const T&... value)
		{
			return (write(reinterpret_cast<const u8*>(&value), sizeof(T)), ...);
		}

		virtual ~BufferWriter() {}
	};

	class trinex_api BufferReader
	{
	public:
		using ReadPos   = usize;
		using PosOffset = i64;
		using Stream    = std::istream;

		usize size();
		BufferReader& position(ReadPos pos);

		virtual bool read(u8* data, usize size)                                          = 0;
		virtual ReadPos position()                                                       = 0;
		virtual BufferReader& offset(PosOffset offset, IOWhence dir = IOWhence::Current) = 0;
		virtual bool is_open() const                                                     = 0;

		template<typename... T>
		inline bool read_primitives(T&... value)
		{
			return (read(reinterpret_cast<u8*>(&value), sizeof(T)), ...);
		}

		template<typename T>
		inline T read_primitive()
		{
			T result;
			trinex_verify(read_primitives(result));
			return result;
		}

		virtual ~BufferReader() {}
	};

	class trinex_api VectorWriterBase : public BufferWriter
	{
	protected:
		static void copy_data(u8* to, const u8* from, usize count);
	};

	class trinex_api VectorReaderBase : public BufferReader
	{
	protected:
		static void copy_data(u8* to, const u8* from, usize count);
	};

	template<typename T>
	class VectorWriter : public VectorWriterBase
	{
	private:
		Array<T>* m_buffer;
		WritePos m_write_pos = 0;

	public:
		VectorWriter(Array<T>* buffer) : m_buffer(buffer) {}

		using VectorWriterBase::position;
		inline WritePos position() override { return m_write_pos; }

		inline VectorWriter& offset(PosOffset offset, IOWhence dir = IOWhence::Current) override
		{
			if (dir == IOWhence::Begin)
				m_write_pos = 0;
			else if (dir == IOWhence::End)
				m_write_pos = m_buffer->size() * sizeof(T);

			m_write_pos += offset;
			return *this;
		}

		bool write(const u8* data, usize size) override
		{
			usize required_size = (m_write_pos + size + sizeof(T) - 1) / sizeof(T);
			if (m_buffer->size() < required_size)
			{
				m_buffer->resize(required_size, T());
			}

			u8* write_to = reinterpret_cast<u8*>(m_buffer->data()) + m_write_pos;
			copy_data(write_to, data, size);
			m_write_pos += size;
			return true;
		}

		bool is_open() const override { return true; }
	};

	template<typename T>
	class VectorReader : public VectorReaderBase
	{
	private:
		const Array<T>* m_buffer;
		ReadPos m_read_pos = 0;

	public:
		VectorReader(const Array<T>* buffer) : m_buffer(buffer) {}

		using VectorReaderBase::position;
		inline ReadPos position() override { return m_read_pos; }

		inline VectorReader& offset(PosOffset offset, IOWhence dir = IOWhence::Current) override
		{
			if (dir == IOWhence::Begin)
				m_read_pos = 0;
			else if (dir == IOWhence::End)
				m_read_pos = m_buffer->size() * sizeof(T);

			m_read_pos += offset;
			return *this;
		}

		bool read(u8* data, usize size) override
		{
			usize required_size = (m_read_pos + size + sizeof(T) - 1) / sizeof(T);
			if (m_buffer->size() < required_size)
			{
				return false;
			}

			const u8* read_from = reinterpret_cast<const u8*>(m_buffer->data()) + m_read_pos;
			copy_data(data, read_from, size);
			m_read_pos += size;
			return true;
		}

		bool is_open() const override { return true; }
	};

} // namespace Trinex
