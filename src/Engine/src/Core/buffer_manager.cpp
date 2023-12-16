#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>

namespace Engine
{
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
}// namespace Engine
