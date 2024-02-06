#include <Core/filesystem/file.hpp>

namespace Engine::VFS
{
    size_t File::size()
    {
        FilePosition pos = tell_read();
        seek_read(0, FileSeekDir::End);
        FilePosition size = tell_read();
        seek_read(static_cast<FileOffset>(pos), FileSeekDir::Begin);
        return size;
    }
}// namespace Engine::VFS
