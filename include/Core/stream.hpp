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
	};
}// namespace Trinex
