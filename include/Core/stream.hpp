#pragma once
#include <Core/enums.hpp>

namespace Trinex
{
	class ENGINE_EXPORT Stream
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
			return read(data, sizeof(data)) == sizeof(data);
		}

		template<typename T>
		u64 write(const T& value)
		{
			const u8* data = reinterpret_cast<const u8*>(&value);
			return write(data, sizeof(data)) == sizeof(data);
		}
	};
}// namespace Trinex
