#include <Core/api_object.hpp>
#include <api_funcs.hpp>

namespace Engine
{
    declare_instance_info_cpp(ApiObject);

    // Zero is default or invalid value of ApiObject in external API
    constructor_cpp(ApiObject)
    {
        _M_ID = 0;
    }


    ObjID ApiObject::id() const
    {
        return _M_ID;
    }

    bool ApiObject::has_object() const
    {
        return _M_ID != 0;
    }

    bool ApiObject::operator==(const ApiObject& obj) const
    {
        return _M_ID == obj._M_ID;
    }

    bool ApiObject::operator!=(const ApiObject& obj) const
    {
        return _M_ID != obj._M_ID;
    }

    bool ApiObject::operator<(const ApiObject& obj) const
    {
        return _M_ID < obj._M_ID;
    }

    bool ApiObject::operator<=(const ApiObject& obj) const
    {
        return _M_ID <= obj._M_ID;
    }

    bool ApiObject::operator>(const ApiObject& obj) const
    {
        return _M_ID > obj._M_ID;
    }

    bool ApiObject::operator>=(const ApiObject& obj) const
    {
        return _M_ID >= obj._M_ID;
    }

    ApiObject::operator ObjID() const
    {
        return _M_ID;
    }

    ApiObject& ApiObject::destroy()
    {
        if (_M_ID)
            destroy_object(_M_ID);
        return *this;
    }

    ApiObject::~ApiObject()
    {
        destroy();
    }
}// namespace Engine
