#include <Core/executable_object.hpp>

namespace Engine
{
    int_t ExecutableObject::execute()
    {
        if(destroy_after_exec)
        {
            delete this;
        }

        return 0;
    }

    SingleTimeExecutableObject::SingleTimeExecutableObject()
    {
        destroy_after_exec = true;
    }
}
