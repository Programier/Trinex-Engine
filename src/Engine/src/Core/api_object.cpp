#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <api.hpp>

namespace Engine
{
    implement_class(ApiObject, "Engine");
    implement_default_initialize_class(ApiObject);

    // Zero is default or invalid value of ApiObjectNoBase in external API
    constructor_cpp(ApiObjectNoBase)
    {
        _M_ID         = 0;
        _M_rhi_object = nullptr;
    }


    Identifier ApiObjectNoBase::id() const
    {
        return _M_ID;
    }

    bool ApiObjectNoBase::has_object() const
    {
        return _M_ID != 0;
    }

    bool ApiObjectNoBase::operator==(const ApiObjectNoBase& obj) const
    {
        return _M_ID == obj._M_ID;
    }

    bool ApiObjectNoBase::operator!=(const ApiObjectNoBase& obj) const
    {
        return _M_ID != obj._M_ID;
    }

    bool ApiObjectNoBase::operator<(const ApiObjectNoBase& obj) const
    {
        return _M_ID < obj._M_ID;
    }

    bool ApiObjectNoBase::operator<=(const ApiObjectNoBase& obj) const
    {
        return _M_ID <= obj._M_ID;
    }

    bool ApiObjectNoBase::operator>(const ApiObjectNoBase& obj) const
    {
        return _M_ID > obj._M_ID;
    }

    bool ApiObjectNoBase::operator>=(const ApiObjectNoBase& obj) const
    {
        return _M_ID >= obj._M_ID;
    }

    ApiObjectNoBase::operator Identifier() const
    {
        return _M_ID;
    }

    ApiObjectNoBase& ApiObjectNoBase::destroy()
    {
        if (_M_ID)
        {
            EngineInstance::instance()->api_interface()->destroy_object(_M_ID);
            _M_ID = 0;
        }

        if (_M_rhi_object)
        {
            delete _M_rhi_object;
            _M_rhi_object = nullptr;
        }
        return *this;
    }

    ApiObjectNoBase::~ApiObjectNoBase()
    {
        destroy();
    }

    const ApiBindingObject& ApiBindingObject::bind(BindingIndex binding, BindingIndex set) const
    {
        if (_M_rhi_binding_object)
        {
            _M_rhi_binding_object->bind(binding, set);
        }
        return *this;
    }
}// namespace Engine
