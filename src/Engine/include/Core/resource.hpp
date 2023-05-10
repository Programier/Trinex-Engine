#pragma once
#include <Core/empty_base.hpp>
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{

    template<typename ResourceType, typename BaseType = EmptyBase>
    class Resource : public BaseType
    {
    protected:
        ResourceType* _M_resources = nullptr;

    public:
        template<typename... Args>
        ResourceType* resources(bool force_create = false, Args&&... args)
        {
            if (_M_resources == nullptr && force_create)
            {
                _M_resources = Object::new_instance<ResourceType>(args...);
            }

            return _M_resources;
        }

        const ResourceType* resources() const
        {
            return _M_resources;
        }

        Resource& delete_resources()
        {
            if (_M_resources)
            {
                Object::begin_destroy(_M_resources);
                _M_resources = nullptr;
            }
            return *this;
        }

        ~Resource()
        {
            delete_resources();
        }
    };

}// namespace Engine
