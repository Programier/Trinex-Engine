#include <Core/file_manager.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/root_filesystem.hpp>

namespace Engine
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

        Flags<FileOpenMode> flags = FileOpenMode::Out;

        if (clear)
        {
            flags(FileOpenMode::Trunc, true);
        }

        _M_file = rootfs()->open(filename, flags);
        return is_open();
    }

    FileWriter& FileWriter::close()
    {
        if (_M_file)
        {
            delete _M_file;
            _M_file = nullptr;
        }
        return *this;
    }


    bool FileWriter::is_open() const
    {
        return _M_file != nullptr && _M_file->is_open();
    }


    FileWriter& FileWriter::clear()
    {
        open(filename(), true);
        return *this;
    }

    const Path& FileWriter::filename() const
    {
        static const Path path;
        if (_M_file)
        {
            return _M_file->path();
        }
        return path;
    }


    bool FileWriter::write(const byte* data, size_t size)
    {
        if (is_open())
            return static_cast<bool>(_M_file->write(data, size)) == size;
        return false;
    }


    FileWriter::WritePos FileWriter::position()
    {
        if (!is_open())
            return 0;
        return _M_file->write_position();
    }

    FileWriter& FileWriter::offset(PosOffset offset, BufferSeekDir dir)
    {
        if (is_open())
        {
            _M_file->write_position(offset, dir);
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
        _M_file = rootfs()->open(path, Flags<FileOpenMode>(FileOpenMode::In));
        return is_open();
    }

    FileReader& FileReader::close()
    {
        if (is_open())
        {
            delete _M_file;
            _M_file = nullptr;
        }
        return *this;
    }

    bool FileReader::is_open() const
    {
        return _M_file != nullptr && _M_file->is_open();
    }

    String FileReader::read_string(size_t len)
    {
        len = glm::min(len, size());
        String result(len, 0);
        read(reinterpret_cast<byte*>(result.data()), len);
        return result;
    }

    Buffer FileReader::read_buffer(size_t len)
    {
        len = glm::min(len, size());
        Buffer result(len, 0);
        read(reinterpret_cast<byte*>(result.data()), len);
        return result;
    }

    const Path& FileReader::filename() const
    {
        static const Path p;
        if (_M_file)
        {
            return _M_file->path();
        }
        return p;
    }


    bool FileReader::read(byte* data, size_t size)
    {
        if (is_open())
            return static_cast<bool>(_M_file->read(data, size)) == size;
        return false;
    }


    FileReader::ReadPos FileReader::position()
    {
        if (!is_open())
            return 0;
        return _M_file->read_position();
    }


    FileReader& FileReader::offset(PosOffset offset, BufferSeekDir dir)
    {
        if (is_open())
        {
            _M_file->read_position(offset, dir);
        }

        return *this;
    }

    FileReader::~FileReader()
    {
        close();
    }
}// namespace Engine
