#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(ApiObject, "Engine");
    implement_default_initialize_class(ApiObject);

    // Zero is default or invalid value of ApiObjectNoBase in external API
    constructor_cpp(ApiObjectNoBase)
    {
        _M_rhi_object = nullptr;
        _M_can_delete = true;
    }


    ApiObjectNoBase& ApiObjectNoBase::destroy()
    {
        if (_M_rhi_object && _M_can_delete)
        {
            delete _M_rhi_object;
            _M_rhi_object = nullptr;
        }
        return *this;
    }

    bool ApiObjectNoBase::has_object() const
    {
        return _M_rhi_object != nullptr;
    }

    ApiObjectNoBase::~ApiObjectNoBase()
    {
        destroy();
    }

    ApiObject& ApiObject::rhi_create()
    {
        destroy();
        return *this;
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
