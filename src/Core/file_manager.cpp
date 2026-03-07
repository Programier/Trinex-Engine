#include <Core/file_manager.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/math/math.hpp>

namespace Trinex
{
	FileWriter::FileWriter() = default;
	FileWriter::FileWriter(const Path& filename, bool clear)
	{
		open(filename, clear);
	}

	FileWriter::FileWriter(FileWriter&&)            = default;
	FileWriter& FileWriter::operator=(FileWriter&&) = default;

	bool FileWriter::open(const Path& filename, bool clear)
	{
		close();

		FileOpenMode flags = FileOpenMode::Out;

		if (!clear)
		{
			flags |= FileOpenMode::Append;
		}

		m_file = rootfs()->open(filename, flags);
		return is_open();
	}

	FileWriter& FileWriter::close()
	{
		if (m_file)
		{
			rootfs()->close(m_file);
			m_file = nullptr;
		}
		return *this;
	}

	bool FileWriter::is_open() const
	{
		return m_file != nullptr;
	}

	bool FileWriter::write(const u8* data, usize size)
	{
		if (is_open())
			return m_file->write(data, size) == size;
		return false;
	}

	FileWriter::WritePos FileWriter::position()
	{
		if (!is_open())
			return 0;
		return m_file->wpos();
	}

	FileWriter& FileWriter::offset(PosOffset offset, BufferSeekDir dir)
	{
		if (is_open())
		{
			m_file->wseek(offset, dir);
		}
		return *this;
	}


	FileWriter::~FileWriter()
	{
		close();
	}


	FileReader::FileReader() = default;
	FileReader::FileReader(const Path& filename)
	{
		open(filename);
	}

	FileReader::FileReader(FileReader&&)            = default;
	FileReader& FileReader::operator=(FileReader&&) = default;


	bool FileReader::open(const Path& path)
	{
		close();
		m_file = rootfs()->open(path, Flags<FileOpenMode>(FileOpenMode::In));
		return is_open();
	}

	FileReader& FileReader::close()
	{
		if (m_file)
		{
			rootfs()->close(m_file);
			m_file = nullptr;
		}
		return *this;
	}

	bool FileReader::is_open() const
	{
		return m_file != nullptr;
	}

	String FileReader::read_string(usize len)
	{
		len = Math::min(len, size());
		String result(len, 0);
		read(reinterpret_cast<u8*>(result.data()), len);
		return result;
	}

	Buffer FileReader::read_buffer(usize len)
	{
		len = Math::min(len, size());
		Buffer result(len, 0);
		read(reinterpret_cast<u8*>(result.data()), len);
		return result;
	}

	bool FileReader::read(u8* data, usize size)
	{
		if (is_open())
			return m_file->read(data, size) == size;
		return false;
	}


	FileReader::ReadPos FileReader::position()
	{
		if (!is_open())
			return 0;
		return m_file->rpos();
	}


	FileReader& FileReader::offset(PosOffset offset, BufferSeekDir dir)
	{
		if (is_open())
		{
			m_file->rseek(offset, dir);
		}

		return *this;
	}

	FileReader::~FileReader()
	{
		close();
	}
}// namespace Trinex
