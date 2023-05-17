#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/serializable_object.hpp>

namespace Engine
{
    bool SerializableObject::archive_process(Archive* archive)
    {
        if (archive == nullptr)
        {
            error_log("SerializableObject: Archive can't be nullptr!");
            return false;
        }

        return true;
    }

    SerializableObject::~SerializableObject()
    {}
}// namespace Engine
