#pragma once
#include <Core/exception.hpp>
#include <Core/export.hpp>


namespace Engine
{
    class BufferWriter;
    class BufferReader;
    class Archive;


    struct ENGINE_EXPORT SerializableObject {
        virtual bool archive_process(Archive* archive);
        virtual ~SerializableObject();
    };

}// namespace Engine
