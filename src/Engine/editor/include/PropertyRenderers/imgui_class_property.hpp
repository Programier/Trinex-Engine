#pragma once

namespace Engine
{
    void render_property(void* object, class Property* prop, bool can_edit);
    void render_object_properties(class Object* object, bool editable = true);
    void render_struct_properties(void* object, class Struct* struct_class, bool editable = true);
}// namespace Engine
