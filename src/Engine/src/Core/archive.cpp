#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class Object* Archive::load_object(const StringView& name)
    {
        return Object::load_object(name);
    }

    Archive::Archive() : _M_reader(nullptr), _M_is_saving(false), _M_process_status(false)
    {}

    Archive::Archive(BufferReader* reader) : _M_is_saving(false)
    {
        _M_reader = reader;
        if (reader == nullptr)
        {
            throw EngineException("Archive: Reader can't be nullptr!");
        }

        _M_process_status = _M_reader->is_open();
    }

    Archive::Archive(BufferWriter* writer) : _M_is_saving(true)
    {
        _M_writer = writer;
        if (writer == nullptr)
        {
            throw EngineException("Archive: Writer can't be nullptr!");
        }

        _M_process_status = _M_writer->is_open();
    }

    Archive::Archive(Archive&& other)
    {
        (*this) = std::move(other);
    }

    Archive& Archive::operator=(Archive&& other)
    {
        if (this == &other)
            return *this;

        _M_reader         = other._M_reader;
        _M_process_status = other._M_process_status;
        _M_is_saving      = other._M_is_saving;

        other._M_process_status = false;
        other._M_reader         = nullptr;
        other._M_is_saving      = false;

        return *this;
    }

    bool Archive::is_saving() const
    {
        return _M_is_saving && _M_writer;
    }

    bool Archive::is_reading() const
    {
        return !_M_is_saving && _M_reader;
    }

    BufferReader* Archive::reader() const
    {
        return _M_is_saving ? nullptr : _M_reader;
    }

    BufferWriter* Archive::writer() const
    {
        return _M_is_saving ? _M_writer : nullptr;
    }

    Archive& Archive::write_data(const byte* data, size_t size)
    {
        if (is_saving())
        {
            _M_writer->write(data, size);
        }

        return *this;
    }

    Archive& Archive::read_data(byte* data, size_t size)
    {
        if (is_reading())
        {
            _M_reader->read(data, size);
        }

        return *this;
    }

    size_t Archive::position() const
    {
        if (is_saving())
        {
            return writer()->position();
        }
        else if (is_reading())
        {
            return reader()->position();
        }

        return 0;
    }

    Archive& Archive::position(size_t position)
    {
        if (is_saving())
        {
            writer()->position(position);
        }
        else if (is_reading())
        {
            reader()->position(position);
        }
        return *this;
    }

    bool Archive::is_open() const
    {
        if (is_saving())
        {
            return writer()->is_open();
        }
        else if (is_reading())
        {
            return reader()->is_open();
        }

        return false;
    }


    ENGINE_EXPORT bool operator&(Archive& ar, String& str)
    {
        size_t size = str.length();
        ar & size;

        if (ar.is_reading())
        {
            str.resize(size);
            ar.read_data(reinterpret_cast<byte*>(str.data()), size);
        }
        else if (ar.is_saving())
        {
            ar.write_data(reinterpret_cast<byte*>(str.data()), size);
        }
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Path& path)
    {
        String str = path.str();
        ar & str;
        if (ar.is_reading())
            path = str;
        return ar;
    }
}// namespace Engine
