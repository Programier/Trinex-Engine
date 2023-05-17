#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    bool BufferWriter::write(const byte* data, size_t size)
    {
        if (is_open())
            return static_cast<bool>(stream().write(reinterpret_cast<const char*>(data), size));
        return false;
    }

    size_t BufferWriter::size()
    {
        if (!is_open())
            return 0;

        auto current_pos = position();
        offset(0, BufferSeekDir::End);
        size_t size = position();
        position(current_pos);
        return size;
    }

    BufferWriter::WritePos BufferWriter::position()
    {
        if (!is_open())
            return 0;
        return static_cast<WritePos>(stream().tellp());
    }


    BufferWriter& BufferWriter::position(BufferWriter::WritePos pos)
    {
        if (is_open())
        {
            stream().seekp(static_cast<std::ostream::pos_type>(pos), std::ios_base::beg);
        }
        return *this;
    }


    BufferWriter& BufferWriter::offset(PosOffset offset, BufferSeekDir dir)
    {
        if (is_open())
        {
            std::ios_base::seekdir _M_dir =
                    (dir == BufferSeekDir::Begin
                             ? std::ios_base::beg
                             : (dir == BufferSeekDir::Current ? std::ios_base::cur : std::ios_base::end));
            stream().seekp(static_cast<std::ostream::pos_type>(offset), _M_dir);
        }
        return *this;
    }

    BufferWriter& BufferWriter::clear()
    {
        logger->error("BufferWriter: Unable to clean buffer: Method '%s' in not overrided!");
        return *this;
    }

    BufferWriter::operator bool()
    {
        return static_cast<bool>(stream());
    }

    bool BufferReader::read(byte* data, size_t size)
    {
        if (is_open())
            return static_cast<bool>(stream().read(reinterpret_cast<char*>(data), size));
        return false;
    }

    size_t BufferReader::size()
    {
        if (!is_open())
            return 0;

        auto current_pos = position();
        offset(0, BufferSeekDir::End);
        size_t size = position();
        position(current_pos);
        return size;
    }

    BufferReader::ReadPos BufferReader::position()
    {
        if (!is_open())
            return 0;
        return static_cast<ReadPos>(stream().tellg());
    }

    BufferReader& BufferReader::position(BufferReader::ReadPos pos)
    {
        if (is_open())
        {
            stream().seekg(static_cast<std::istream::pos_type>(pos), std::ios_base::beg);
        }
        return *this;
    }

    BufferReader& BufferReader::offset(BufferReader::PosOffset offset, BufferSeekDir dir)
    {
        if (is_open())
        {
            std::ios_base::seekdir _M_dir =
                    (dir == BufferSeekDir::Begin
                             ? std::ios_base::beg
                             : (dir == BufferSeekDir::Current ? std::ios_base::cur : std::ios_base::end));
            stream().seekg(static_cast<std::istream::pos_type>(offset), _M_dir);
        }
        return *this;
    }

    BufferReader::operator bool()
    {
        return static_cast<bool>(stream());
    }


    bool TextBufferReader::read_line(String& line, char delimiter)
    {
        if (is_open())
            return static_cast<bool>(std::getline(stream(), line, delimiter));
        return false;
    }

    String TextBufferReader::read_line(char separator)
    {
        String result;
        if (!read_line(result, separator))
        {
            return "";
        }
        return result;
    }

    Archive::Archive(BufferReader* reader) : _M_is_saving(false)
    {
        _M_reader = reader;
        if (reader == nullptr)
        {
            throw EngineException("Archive: Reader can't be nullptr!");
        }
    }

    Archive::Archive(BufferWriter* writer) : _M_is_saving(true)
    {
        _M_writer = writer;
        if (writer == nullptr)
        {
            throw EngineException("Archive: Writer can't be nullptr!");
        }
    }

    bool Archive::is_saving() const
    {
        return _M_is_saving;
    }

    bool Archive::is_reading() const
    {
        return !_M_is_saving;
    }

    BufferReader* Archive::reader() const
    {
        return _M_is_saving ? nullptr : _M_reader;
    }

    BufferWriter* Archive::writer() const
    {
        return _M_is_saving ? _M_writer : nullptr;
    }
}// namespace Engine
