#include <Core/filesystem/native_file.hpp>
#include <Core/filesystem/native_file_system.hpp>

namespace Engine::VFS
{
	NativeFile::NativeFile(NativeFileSystem* fs, const Path& path, std::fstream&& stream)
	    : m_fs(fs), m_path(path), m_stream(std::move(stream))
	{}

	NativeFile::~NativeFile()
	{
		m_stream.close();
	}

	const Path& NativeFile::path() const
	{
		return m_path;
	}

	static FORCE_INLINE std::ios_base::seekdir direction_of(FileSeekDir dir)
	{
		switch (dir)
		{
			case FileSeekDir::Begin: return std::ios_base::beg;
			case FileSeekDir::Current: return std::ios_base::cur;
			case FileSeekDir::End: return std::ios_base::end;
			default: return std::ios_base::beg;
		}
	}

	FileSystem* NativeFile::filesystem() const
	{
		return m_fs;
	}

	NativeFile::FilePosition NativeFile::wseek(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekp(offset, direction_of(dir));
		return m_stream.tellp();
	}

	NativeFile::FilePosition NativeFile::wpos()
	{
		return m_stream.tellp();
	}

	NativeFile::FilePosition NativeFile::rseek(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekg(offset, direction_of(dir));
		return m_stream.tellg();
	}

	NativeFile::FilePosition NativeFile::rpos()
	{
		return m_stream.tellg();
	}

	size_t NativeFile::read(byte* buffer, size_t size)
	{
		m_stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(size));

		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}

	size_t NativeFile::write(const byte* buffer, size_t size)
	{
		m_stream.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(size));
		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}

	ReadOnlyNativeFile::ReadOnlyNativeFile(NativeFileSystem* fs, const Path& path, std::fstream&& stream)
	    : NativeFile(fs, path, std::move(stream))
	{}

	ReadOnlyNativeFile::FilePosition ReadOnlyNativeFile::wseek(FileOffset offset, FileSeekDir dir)
	{
		return 0;
	}

	ReadOnlyNativeFile::FilePosition ReadOnlyNativeFile::wpos()
	{
		return 0;
	}

	size_t ReadOnlyNativeFile::write(const byte* buffer, size_t size)
	{
		return 0;
	}

	WriteOnlyNativeFile::WriteOnlyNativeFile(NativeFileSystem* fs, const Path& path, std::fstream&& stream)
	    : NativeFile(fs, path, std::move(stream))
	{}

	WriteOnlyNativeFile::FilePosition WriteOnlyNativeFile::rseek(FileOffset offset, FileSeekDir dir)
	{
		return 0;
	}

	WriteOnlyNativeFile::FilePosition WriteOnlyNativeFile::rpos()
	{
		return 0;
	}

	size_t WriteOnlyNativeFile::read(byte* buffer, size_t size)
	{
		return 0;
	}
}// namespace Engine::VFS
