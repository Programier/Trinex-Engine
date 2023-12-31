#include <Core/object.hpp>


namespace Engine
{
    void Object::mark_internal_objects()
    {
        const char* internal_objects_list[] = {
                "Engine",
                "Engine::GBuffer",
                "Engine::SceneColorOutput"
        };


        size_t count = sizeof(internal_objects_list) / sizeof(const char*);

        for (size_t i = 0; i < count; i++)
        {
            Object* object = Object::find_object(internal_objects_list[i]);
            if (object)
            {
                object->flag(Object::IsInternal, true);
            }
        }
    }

}// namespace Engine
