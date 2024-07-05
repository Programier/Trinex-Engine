#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/serializable_object.hpp>

namespace Engine
{
    bool SerializableObject::archive_process(Archive& archive)
    {
        return true;
    }

    SerializableObject::~SerializableObject()
    {}
}// namespace Engine
