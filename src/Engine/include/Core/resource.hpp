#pragma once
#include <Core/object.hpp>
#include <Core/structures.hpp>

namespace Engine
{

    template<typename RT, typename BaseType = EmptyClass>
    class Resource : public BaseType
    {
    public:
        using ResourceType = RT;

    protected:
        ResourceType* _M_resources = nullptr;

    public:
        Resource()
        {}

        Resource(const Resource& resource)
        {
            (*this) = resource;
        }

        Resource(Resource&& resource)
        {
            (*this) = std::move(resource);
        }

        Resource& operator=(const Resource& resource)
        {
            if (this == &resource)
                return *this;

            delete_resources();
            if constexpr (std::is_base_of_v<Object, ResourceType>)
            {
                _M_resources = _M_resources->copy();
            }
            else if constexpr (std::is_copy_assignable_v<ResourceType>)
            {
                (*_M_resources) = (*resource._M_resources);
            }
            else if constexpr (std::is_copy_constructible_v<ResourceType>)
            {
                _M_resources = Object::new_instance<ResourceType>(*resource._M_resources);
            }
            else
            {
                throw EngineException("Resource type is not copyable");
            }

            return *this;
        }

        Resource& operator=(const Resource&& resource)
        {
            if (this == &resource)
                return *this;

            delete_resources();
            _M_resources          = resource._M_resources;
            resource._M_resources = nullptr;
            return *this;
        }

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
