#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>

namespace Engine
{
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

    ENGINE_EXPORT bool operator&(Archive& ar, String& str)
    {
        size_t size = str.length();
        ar& size;

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
}// namespace Engine
