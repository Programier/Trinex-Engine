#include <Core/file_reader.hpp>


namespace Engine
{
    FileReader::FileReader() = default;

    FileReader::FileReader(const String& filename)
    {
        open(filename);
    }

    size_t FileReader::max_size(size_t size)
    {
        return glm::min(size, _M_size - _M_position);
    }

    FileReader& FileReader::open(const String& filename)
    {
        close();

        _M_file.open(filename, std::ios_base::binary);
        _M_file.seekg(0, std::ios_base::end);
        _M_size = static_cast<size_t>(_M_file.tellg());
        _M_file.seekg(0, std::ios_base::beg);
        return *this;
    }

    FileReader& FileReader::close()
    {
        _M_file.close();
        _M_size = 0;
        _M_position = 0;

        return *this;
    }

    bool FileReader::is_open() const
    {
        return _M_file.is_open();
    }

    std::size_t FileReader::size()
    {
        return _M_size;
    }

    FileReader::read_pos FileReader::position() const
    {
        return _M_position;
    }

    FileReader& FileReader::position(read_pos pos)
    {
        if (is_open())
        {
            _M_position = pos;
            _M_file.seekg(glm::min(pos, _M_size), std::ios_base::beg);
        }

        return *this;
    }

    bool FileReader::read(byte* to, size_t size)
    {
        size = max_size(size);

        if (size != 0)
        {
            _M_position += size;
            return static_cast<bool>(_M_file.read(reinterpret_cast<char*>(to), size));
        }

        return false;
    }

    bool FileReader::read(FileBuffer& buffer, size_t size)
    {
        size = max_size(size);
        buffer.clear();
        buffer.resize(size);
        return read(buffer.data(), size);
    }
}// namespace Engine
