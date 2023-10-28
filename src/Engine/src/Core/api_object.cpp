#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(ApiObject, "Engine");
    implement_default_initialize_class(ApiObject);
    implement_class(ApiBindingObject, "Engine");
    implement_default_initialize_class(ApiBindingObject);

    // Zero is default or invalid value of ApiObjectNoBase in external API
    ApiObject::ApiObject()
    {
        _M_rhi_object = nullptr;
        _M_can_delete = true;
    }

    ApiObject& ApiObject::rhi_destroy()
    {
        if (_M_rhi_object && _M_can_delete)
        {
            delete _M_rhi_object;
            _M_rhi_object = nullptr;
        }
        return *this;
    }

    bool ApiObject::has_object() const
    {
        return _M_rhi_object != nullptr;
    }

    ApiObject::~ApiObject()
    {
        rhi_destroy();
    }

    const ApiBindingObject& ApiBindingObject::rhi_bind(BindingIndex binding, BindingIndex set) const
    {
        if (_M_rhi_binding_object)
        {
            _M_rhi_binding_object->bind(binding, set);
        }
        return *this;
    }
}// namespace Engine
