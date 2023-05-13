#pragma once
#include <Core/export.hpp>

namespace Engine
{
    class BufferWriter;
    class BufferReader;

    struct ENGINE_EXPORT SerializableObject {
        virtual bool serialize(BufferWriter* writer) const;
        virtual bool deserialize(BufferReader* reader);
        virtual ~SerializableObject();
    };
}// namespace Engine
