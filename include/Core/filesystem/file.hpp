#pragma once
#include <Core/enums.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/flags.hpp>

namespace Engine::VFS
{
	class FileSystem;

	class ENGINE_EXPORT File
	{
	public:
		using FileOffset   = i64;
		using FilePosition = u64;

		virtual ~File() = default;

		virtual FileSystem* filesystem() const                         = 0;
		virtual FilePosition rseek(FileOffset offset, FileSeekDir dir) = 0;
		virtual FilePosition rpos()                                    = 0;
		virtual FilePosition wseek(FileOffset offset, FileSeekDir dir) = 0;
		virtual FilePosition wpos()                                    = 0;
		virtual usize read(u8* buffer, usize size)                     = 0;
		virtual usize write(const u8* buffer, usize size)              = 0;

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
}// namespace Engine::VFS
