#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/serializable_object.hpp>

namespace Engine
{
    bool SerializableObject::serialize(BufferWriter* writer)
    {
        if (!writer)
        {
            logger->error("SerializableObject: Failed to serialize object. Writer is nullptr!");
            return false;
        }

        if (!writer->is_open())
        {
            logger->error("SerializableObject: Failed to serialize object. Writer is not open!");
            return false;
        }

        return true;
    }

    bool SerializableObject::deserialize(BufferReader* reader)
    {
        if (!reader)
        {
            logger->error("SerializableObject: Failed to deserialize object. Reader is nullptr!");
            return false;
        }

        if (!reader->is_open())
        {
            logger->error("SerializableObject: Failed to deserialize object. Reader is not open!");
            return false;
        }

        return true;
    }

    SerializableObject::~SerializableObject()
    {}
}// namespace Engine
