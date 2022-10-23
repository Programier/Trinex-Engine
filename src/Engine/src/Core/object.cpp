#include <Core/object.hpp>
#include <api_funcs.hpp>

namespace Engine
{
    // Zero is default or invalid value of object in external API
    Object::Object() : _M_ID(0){};

    Object::Object(const Object& object) : Object()
    {
        link_object(object._M_ID, _M_ID);
    }

    Object::Object(Object&& obj) : Object()
    {
        _M_ID = obj._M_ID;
        obj._M_ID = 0;
    }

    Object& Object::operator=(Object&& obj)
    {
        if (this == &obj)
            return *this;
        _M_ID = obj._M_ID;
        obj._M_ID = 0;
        return *this;
    }

    Object& Object::operator=(const Object& obj)
    {
        if (this == &obj)
            return *this;

        link_object(obj._M_ID, _M_ID);
        return *this;
    }

    ObjID Object::id() const
    {
        return _M_ID;
    }

    bool Object::has_object() const
    {
        return _M_ID != 0;
    }

    bool Object::operator==(const Object& obj)
    {
        return _M_ID == obj._M_ID;
    }

    bool Object::operator!=(const Object& obj)
    {
        return _M_ID != obj._M_ID;
    }

    Object& Object::destroy()
    {
        if (_M_ID)
            destroy_object(_M_ID);
        return *this;
    }

    Object::~Object()
    {
        destroy();
    }
}// namespace Engine
