#pragma once
#include <Core/enums.hpp>
#include <Core/ref_counted.hpp>

namespace Trinex
{
	class ENGINE_EXPORT Stream : public RefCounted
	{
	public:
		virtual ~Stream() = default;

		virtual bool seekable() const = 0;
		virtual bool readable() const = 0;
		virtual bool writable() const = 0;

		virtual u64 offset(i64 value, IOWhence whence = IOWhence::Current) = 0;
		virtual u64 offset() const                                         = 0;
		virtual usize read(void* buffer, usize size)                       = 0;
		virtual usize write(const void* buffer, usize size)                = 0;
		virtual bool flush()                                               = 0;

		template<typename T>
		bool read(T& value)
		{
			u8* data = reinterpret_cast<u8*>(&value);
			return read(data, sizeof(T)) == sizeof(T);
		}

		template<typename T>
		u64 write(const T& value)
		{
			const u8* data = reinterpret_cast<const u8*>(&value);
			return write(data, sizeof(T)) == sizeof(T);
		}
	};

	class ENGINE_EXPORT BlobStream : public Stream
	{
	private:
		usize m_offset = 0;

	protected:
		virtual u8* data()              = 0;
		virtual usize size() const      = 0;
		virtual bool resize(usize size) = 0;

	public:
		bool seekable() const override;
		bool readable() const override;
		bool flush() override;

		u64 offset(i64 value, IOWhence whence = IOWhence::Current) override;
		u64 offset() const override;

		usize read(void* buffer, usize size) override;
		usize write(const void* buffer, usize size) override;
	};

	template<typename T>
	concept StreamMemoryBlob = requires(T& buffer) {
		buffer.data();
		buffer.size();
	};

	template<typename T>
	concept StreamResizableBlob = requires(T& buffer, usize size) { buffer.resize(size); };

	class MemoryStream final : public BlobStream
	{
	private:
		u8* m_memory;
		usize m_size;

	public:
		MemoryStream(void* memory, usize size) : m_memory(static_cast<u8*>(memory)), m_size(size) {}

		bool writable() const override { return true; }
		u8* data() override { return m_memory; }
		usize size() const override { return m_size; }
		bool resize(usize size) override { return false; }
	};

	template<StreamMemoryBlob Buffer>
	class BufferStream final : public BlobStream
	{
	private:
		Buffer& m_buffer;

	public:
		BufferStream(Buffer& buffer) : m_buffer(buffer) {}

		bool writable() const override { return true; }

		u8* data() override { return reinterpret_cast<u8*>(m_buffer.data()); }

		usize size() const override { return m_buffer.size() * sizeof(typename Buffer::value_type); }

		bool resize(usize size) override
		{
			if constexpr (StreamResizableBlob<Buffer>)
			{
				const usize element_size = sizeof(typename Buffer::value_type);
				const usize count        = (size + element_size - 1) / element_size;

				m_buffer.resize(count);
				return true;
			}
			else
			{
				return false;
			}
		}
	};

}// namespace Trinex
