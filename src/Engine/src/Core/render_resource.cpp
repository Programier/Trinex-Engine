#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/render_resource.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderResource, "Engine");
    implement_default_initialize_class(RenderResource);
    implement_class(BindedRenderResource, "Engine");
    implement_default_initialize_class(BindedRenderResource);

    // Zero is default or invalid value of RenderResourceNoBase in external API
    RenderResource::RenderResource()
    {
        _M_rhi_object = nullptr;
        _M_can_delete = true;
    }

    RenderResource& RenderResource::rhi_create()
    {
        return *this;
    }

    RenderResource& RenderResource::init_resource()
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
        {}

        int_t execute()
        {
            delete object;
            return sizeof(DestroyRenderResource);
        }
    };

    RenderResource& RenderResource::rhi_destroy()
    {
        if (_M_rhi_object && _M_can_delete)
        {
            if (Thread::this_thread() == engine_instance->thread(ThreadType::RenderThread))
            {
                delete _M_rhi_object;
            }
            else
            {
                engine_instance->thread(ThreadType::RenderThread)
                        ->insert_new_task<DestroyRenderResource>(_M_rhi_object);
            }

            _M_rhi_object = nullptr;
        }
        return *this;
    }

    bool RenderResource::has_object() const
    {
        return _M_rhi_object != nullptr;
    }

    RenderResource::~RenderResource()
    {
        rhi_destroy();
    }

    const BindedRenderResource& BindedRenderResource::rhi_bind(BindLocation location) const
    {
        if (_M_rhi_binding_object)
        {
            _M_rhi_binding_object->bind(location);
        }
        return *this;
    }

    InitRenderResourceTask::InitRenderResourceTask(RenderResource* object) : resource(object)
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
