#pragma once

namespace Engine
{
    void render_object_property(class Object* object, class Property* prop, bool can_edit);
    void render_object_properties(class Object* object, bool editable = true);
}// namespace Engine
