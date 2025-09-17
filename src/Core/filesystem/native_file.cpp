#include <Core/filesystem/native_file.hpp>

namespace Engine::VFS
{
	NativeFile::NativeFile(const Path& path, const Path& native_path, std::fstream&& stream, bool is_read_only)
	    : m_path(path), m_native_path(native_path), m_stream(std::move(stream)), m_is_read_only(is_read_only)
	{}

	const Path& NativeFile::path() const
	{
		return m_path;
	}

	const Path& NativeFile::native_path() const
	{
		return m_native_path;
	}

	bool NativeFile::is_read_only() const
	{
		return m_is_read_only;
	}

	void NativeFile::close()
	{
		m_stream.close();
		trx_delete this;
	}

	bool NativeFile::is_open() const
	{
		return m_stream.is_open();
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

	NativeFile::FilePosition NativeFile::write_position(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekp(offset, direction_of(dir));
		return write_position();
	}

	NativeFile::FilePosition NativeFile::write_position()
	{
		return m_stream.tellp();
	}

	NativeFile::FilePosition NativeFile::read_position(FileOffset offset, FileSeekDir dir)
	{
		m_stream.seekg(offset, direction_of(dir));
		return read_position();
	}

	NativeFile::FilePosition NativeFile::read_position()
	{
		return m_stream.tellg();
	}

	size_t NativeFile::read(byte* buffer, size_t size)
	{
		if (!is_open())
			return 0;

		m_stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(size));

		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}

	size_t NativeFile::write(const byte* buffer, size_t size)
	{
		if (!is_open() || is_read_only())
		{
			return 0;
		}

		m_stream.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(size));
		if (m_stream)
		{
			return size;
		}

		return static_cast<size_t>(m_stream.gcount());
	}
}// namespace Engine::VFS
