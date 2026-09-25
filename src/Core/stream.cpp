#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/stream.hpp>

namespace Trinex
{
	bool BlobStream::seekable() const
	{
		return true;
	}

	bool BlobStream::readable() const
	{
		return true;
	}

	bool BlobStream::flush()
	{
		return true;
	}

	u64 BlobStream::offset(i64 value, IOWhence whence)
	{
		i64 base = 0;

		switch (whence)
		{
			case IOWhence::Begin: base = 0; break;
			case IOWhence::Current: base = static_cast<i64>(m_offset); break;
			case IOWhence::End: base = static_cast<i64>(size()); break;
		}

		const i64 result = base + value;
		m_offset         = result >= 0 ? static_cast<usize>(result) : 0;
		return m_offset;
	}

	u64 BlobStream::offset() const
	{
		return m_offset;
	}

	usize BlobStream::read(void* buffer, usize count)
	{
		if (!buffer || count == 0)
			return 0;

		const usize blob_size = size();

		if (m_offset >= blob_size)
			return 0;

		const usize available = blob_size - m_offset;
		const usize read_size = Math::min(count, available);

		std::memcpy(buffer, data() + m_offset, read_size);
		m_offset += read_size;
		return read_size;
	}

	usize BlobStream::write(const void* buffer, usize count)
	{
		if (!buffer || count == 0)
			return 0;

		const usize required = m_offset + count;

		if (required > size())
		{
			if (!resize(required))
			{
				if (m_offset >= size())
					return 0;

				count = Math::min(count, size() - m_offset);
			}
		}

		std::memcpy(data() + m_offset, buffer, count);
		m_offset += count;

		return count;
	}

}// namespace Trinex
