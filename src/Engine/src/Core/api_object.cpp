#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/thread.hpp>
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

    ApiObject& ApiObject::rhi_create()
    {
        return *this;
    }

    ApiObject& ApiObject::init_resource()
    {
        Thread* render_thread = engine_instance->thread(ThreadType::RenderThread);
        if (Thread::this_thread() == render_thread)
        {
            rhi_create();
        }
        else
        {
            render_thread->insert_new_task<InitRenderResourceTask>(this);
        }

        return *this;
    }


    struct DestroyRenderResource : public ExecutableObject {
        RHI_Object* object;

        DestroyRenderResource(RHI_Object* obj) : object(obj)
        {
        }

        int_t execute()
        {
            delete object;
            return sizeof(DestroyRenderResource);
        }
    };

    ApiObject& ApiObject::rhi_destroy()
    {
        if (_M_rhi_object && _M_can_delete)
        {
            if (Thread::this_thread() == engine_instance->thread(ThreadType::RenderThread))
            {
                delete _M_rhi_object;
            }
            else
            {
                engine_instance->thread(ThreadType::RenderThread)->insert_new_task<DestroyRenderResource>(_M_rhi_object);
            }

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

    const ApiBindingObject& ApiBindingObject::rhi_bind(BindLocation location) const
    {
        if (_M_rhi_binding_object)
        {
            _M_rhi_binding_object->bind(location);
        }
        return *this;
    }

    InitRenderResourceTask::InitRenderResourceTask(ApiObject* object) : resource(object)
    {}

    int_t InitRenderResourceTask::execute()
    {
        if (resource)
        {
            resource->rhi_create();
        }

        return sizeof(InitRenderResourceTask);
    }
}// namespace Engine
