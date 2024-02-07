#include <Core/filesystem/native_file.hpp>

namespace Engine::VFS
{
    NativeFile::NativeFile(const Path& path, const Path& native_path, std::fstream&& stream, bool is_read_only)
        : _M_path(path), _M_native_path(native_path), _M_stream(std::move(stream)), _M_is_read_only(is_read_only)
    {}

    const Path& NativeFile::path() const
    {
        return _M_path;
    }

    const Path& NativeFile::native_path() const
    {
        return _M_native_path;
    }

    bool NativeFile::is_read_only() const
    {
        return _M_is_read_only;
    }

    void NativeFile::close()
    {
        _M_stream.close();
        delete this;
    }

    bool NativeFile::is_open() const
    {
        return _M_stream.is_open();
    }

    static FORCE_INLINE std::ios_base::seekdir direction_of(FileSeekDir dir)
    {
        switch (dir)
        {
            case FileSeekDir::Begin:
                return std::ios_base::beg;
            case FileSeekDir::Current:
                return std::ios_base::cur;
            case FileSeekDir::End:
                return std::ios_base::end;
            default:
                return std::ios_base::beg;
        }
    }

    NativeFile::FilePosition NativeFile::write_position(FileOffset offset, FileSeekDir dir)
    {
        _M_stream.seekp(offset, direction_of(dir));
        return write_position();
    }

    NativeFile::FilePosition NativeFile::write_position()
    {
        return _M_stream.tellp();
    }

    NativeFile::FilePosition NativeFile::read_position(FileOffset offset, FileSeekDir dir)
    {
        _M_stream.seekg(offset, direction_of(dir));
        return read_position();
    }

    NativeFile::FilePosition NativeFile::read_position()
    {
        return _M_stream.tellg();
    }

    size_t NativeFile::read(byte* buffer, size_t size)
    {
        if (!is_open())
            return 0;

        _M_stream.read(reinterpret_cast<char*>(buffer), static_cast<std::streamsize>(size));

        if (_M_stream)
        {
            return size;
        }

        return static_cast<size_t>(_M_stream.gcount());
    }

    size_t NativeFile::write(const byte* buffer, size_t size)
    {
        if (!is_open() || is_read_only())
        {
            return 0;
        }

        _M_stream.write(reinterpret_cast<const char*>(buffer), static_cast<std::streamsize>(size));
        if (_M_stream)
        {
            return size;
        }

        return static_cast<size_t>(_M_stream.gcount());
    }
}// namespace Engine::VFS
