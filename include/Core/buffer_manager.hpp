#pragma once
#include <Core/enums.hpp>
#include <Core/exception.hpp>

namespace Engine
{
	class ENGINE_EXPORT BufferWriter
	{
	public:
		using Stream	= std::ostream;
		using WritePos	= size_t;
		using PosOffset = int64_t;


		size_t size();
		BufferWriter& position(WritePos pos);

		virtual bool write(const byte* data, size_t size)										   = 0;
		virtual WritePos position()																   = 0;
		virtual BufferWriter& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) = 0;
		virtual bool is_open() const															   = 0;
		virtual BufferWriter& clear()															   = 0;


		template<typename... T>
		FORCE_INLINE bool write_primitives(const T&... value)
		{
			return (write(reinterpret_cast<const byte*>(&value), sizeof(T)), ...);
		}

		virtual ~BufferWriter()
		{}
	};


	class ENGINE_EXPORT BufferReader
	{
	public:
		using ReadPos	= size_t;
		using PosOffset = int64_t;
		using Stream	= std::istream;

		size_t size();
		BufferReader& position(ReadPos pos);

		virtual bool read(byte* data, size_t size)												   = 0;
		virtual ReadPos position()																   = 0;
		virtual BufferReader& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) = 0;
		virtual bool is_open() const															   = 0;


		template<typename... T>
		FORCE_INLINE bool read_primitives(T&... value)
		{
			return (read(reinterpret_cast<byte*>(&value), sizeof(T)), ...);
		}

		template<typename T>
		FORCE_INLINE T read_primitive()
		{
			T result;
			if (read_primitives(result))
			{
				throw EngineException("Failed to read primitive");
			}

			return result;
		}

		virtual ~BufferReader()
		{}
	};


	class ENGINE_EXPORT VectorWriterBase : public BufferWriter
	{
	protected:
		static void copy_data(byte* to, const byte* from, size_t count);
	};

	class ENGINE_EXPORT VectorReaderBase : public BufferReader
	{
	protected:
		static void copy_data(byte* to, const byte* from, size_t count);
	};

	template<typename T, typename AllocatorType>
	class VectorWriter : public VectorWriterBase
	{
	private:
		Vector<T, AllocatorType>* m_buffer;
		WritePos m_write_pos = 0;

	public:
		VectorWriter(Vector<T, AllocatorType>* buffer) : m_buffer(buffer)
		{}

		using VectorWriterBase::position;
		FORCE_INLINE WritePos position() override
		{
			return m_write_pos;
		}

		FORCE_INLINE VectorWriter& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) override
		{
			if (dir == BufferSeekDir::Begin)
				m_write_pos = 0;
			else if (dir == BufferSeekDir::End)
				m_write_pos = m_buffer->size() * sizeof(T);

			m_write_pos += offset;
			return *this;
		}

		bool write(const byte* data, size_t size) override
		{
			size_t required_size = (m_write_pos + size + sizeof(T) - 1) / sizeof(T);
			if (m_buffer->size() < required_size)
			{
				m_buffer->resize(required_size, T());
			}

			byte* write_to = reinterpret_cast<byte*>(m_buffer->data()) + m_write_pos;
			copy_data(write_to, data, size);
			m_write_pos += size;
			return true;
		}

		bool is_open() const override
		{
			return true;
		}

		VectorWriter& clear() override
		{
			m_buffer->clear();
			return *this;
		}
	};

	template<typename T, typename AllocatorType>
	class VectorReader : public VectorReaderBase
	{
	private:
		const Vector<T, AllocatorType>* m_buffer;
		ReadPos m_read_pos = 0;

	public:
		VectorReader(const Vector<T, AllocatorType>* buffer) : m_buffer(buffer)
		{}

		using VectorReaderBase::position;
		FORCE_INLINE ReadPos position() override
		{
			return m_read_pos;
		}

		FORCE_INLINE VectorReader& offset(PosOffset offset, BufferSeekDir dir = BufferSeekDir::Current) override
		{
			if (dir == BufferSeekDir::Begin)
				m_read_pos = 0;
			else if (dir == BufferSeekDir::End)
				m_read_pos = m_buffer->size() * sizeof(T);

			m_read_pos += offset;
			return *this;
		}

		bool read(byte* data, size_t size) override
		{
			size_t required_size = (m_read_pos + size + sizeof(T) - 1) / sizeof(T);
			if (m_buffer->size() < required_size)
			{
				return false;
			}

			const byte* read_from = reinterpret_cast<const byte*>(m_buffer->data()) + m_read_pos;
			copy_data(data, read_from, size);
			m_read_pos += size;
			return true;
		}

		bool is_open() const override
		{
			return true;
		}
	};

}// namespace Engine
