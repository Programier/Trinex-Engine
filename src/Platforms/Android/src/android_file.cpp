#include <android_file.hpp>
#include <android_file_system.hpp>

namespace Engine::VFS
{
	AndroidFile::AndroidFile(AndroidFileSystem* fs, const Path& path, std::fstream&& stream)
	    : m_fs(fs), m_path(path), m_stream(std::move(stream))
	{}

	AndroidFile::~AndroidFile()
	{
		m_stream.close();
	}

	const Path& AndroidFile::path() const
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

	FileSystem* AndroidFile::filesystem() const
	{
		return m_fs;
	}

	AndroidFile::FilePosition AndroidFile::wseek(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekp(offset, direction_of(dir));
		return m_stream.tellp();
	}

	AndroidFile::FilePosition AndroidFile::wpos()
	{
		return m_stream.tellp();
	}

	AndroidFile::FilePosition AndroidFile::rseek(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekg(offset, direction_of(dir));
		return m_stream.tellg();
	}

	AndroidFile::FilePosition AndroidFile::rpos()
	{
		return m_stream.tellg();
	}

	size_t AndroidFile::read(byte* buffer, size_t size)
	{
		m_stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(size));

		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}

	size_t AndroidFile::write(const byte* buffer, size_t size)
	{
		m_stream.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(size));
		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}
}// namespace Engine::VFS
