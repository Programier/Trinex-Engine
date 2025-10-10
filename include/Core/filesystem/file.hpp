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
		using FileOffset   = int64_t;
		using FilePosition = uint64_t;

		virtual ~File() = default;

		virtual FileSystem* filesystem() const                         = 0;
		virtual FilePosition rseek(FileOffset offset, FileSeekDir dir) = 0;
		virtual FilePosition rpos()                                    = 0;
		virtual FilePosition wseek(FileOffset offset, FileSeekDir dir) = 0;
		virtual FilePosition wpos()                                    = 0;
		virtual size_t read(byte* buffer, size_t size)                 = 0;
		virtual size_t write(const byte* buffer, size_t size)          = 0;

		template<typename T>
		bool read(T& value)
		{
			byte* data = reinterpret_cast<byte*>(&value);
			return read(data, sizeof(data)) == sizeof(data);
		}

		template<typename T>
		uint64_t write(const T& value)
		{
			const byte* data = reinterpret_cast<const byte*>(&value);
			return write(data, sizeof(data)) == sizeof(data);
		}
	};
}// namespace Engine::VFS
