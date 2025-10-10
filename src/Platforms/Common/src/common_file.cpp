#include <common_file.hpp>
#include <common_file_system.hpp>

namespace Engine::VFS
{
	CommonFile::CommonFile(CommonFileSystem* fs, const Path& path, std::fstream&& stream)
	    : m_fs(fs), m_path(path), m_stream(std::move(stream))
	{}

	CommonFile::~CommonFile()
	{
		m_stream.close();
	}

	const Path& CommonFile::path() const
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

	FileSystem* CommonFile::filesystem() const
	{
		return m_fs;
	}

	CommonFile::FilePosition CommonFile::wseek(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekp(offset, direction_of(dir));
		return m_stream.tellp();
	}

	CommonFile::FilePosition CommonFile::wpos()
	{
		return m_stream.tellp();
	}

	CommonFile::FilePosition CommonFile::rseek(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekg(offset, direction_of(dir));
		return m_stream.tellg();
	}

	CommonFile::FilePosition CommonFile::rpos()
	{
		return m_stream.tellg();
	}

	size_t CommonFile::read(byte* buffer, size_t size)
	{
		m_stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(size));

		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}

	size_t CommonFile::write(const byte* buffer, size_t size)
	{
		m_stream.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(size));
		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}

	ReadOnlyCommonFile::ReadOnlyCommonFile(CommonFileSystem* fs, const Path& path, std::fstream&& stream)
	    : CommonFile(fs, path, std::move(stream))
	{}

	ReadOnlyCommonFile::FilePosition ReadOnlyCommonFile::wseek(FileOffset offset, FileSeekDir dir)
	{
		return 0;
	}

	ReadOnlyCommonFile::FilePosition ReadOnlyCommonFile::wpos()
	{
		return 0;
	}

	size_t ReadOnlyCommonFile::write(const byte* buffer, size_t size)
	{
		return 0;
	}

	WriteOnlyCommonFile::WriteOnlyCommonFile(CommonFileSystem* fs, const Path& path, std::fstream&& stream)
	    : CommonFile(fs, path, std::move(stream))
	{}

	WriteOnlyCommonFile::FilePosition WriteOnlyCommonFile::rseek(FileOffset offset, FileSeekDir dir)
	{
		return 0;
	}

	WriteOnlyCommonFile::FilePosition WriteOnlyCommonFile::rpos()
	{
		return 0;
	}

	size_t WriteOnlyCommonFile::read(byte* buffer, size_t size)
	{
		return 0;
	}
}// namespace Engine::VFS
