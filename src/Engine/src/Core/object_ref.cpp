#include <Core/logger.hpp>
#include <Core/object_ref.hpp>

namespace Engine
{

    bool ObjectReferenceBase::serialize_name(BufferWriter* writer, const String& name) const
    {
        if (!writer->write(name))
        {
            error_log("ObjectReferenceBase: Failed to serialize object reference name!");
            return false;
        }

        return true;
    }

    bool ObjectReferenceBase::deserialize_name(BufferReader* reader, String& name)
    {
        if (!reader->read(name))
        {
            error_log("ObjectReferenceBase: Failed to deserialize object reference name!");
            return false;
        }

        return true;
    }
}// namespace Engine
