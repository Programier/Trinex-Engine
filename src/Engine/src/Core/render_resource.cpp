#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/render_resource.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderResource, "Engine", 0);
    implement_default_initialize_class(RenderResource);
    implement_class(BindedRenderResource, "Engine", 0);
    implement_default_initialize_class(BindedRenderResource);

    struct DestroyRenderResourceTask : public ExecutableObject {
        RHI_Object* object;
        RHI* rhi;

        DestroyRenderResourceTask(RHI_Object* obj) : object(obj), rhi(engine_instance->rhi())
        {}

        int_t execute()
        {
            rhi->destroy_object(object);
            return sizeof(DestroyRenderResourceTask);
        }
    };


    void RenderResource::DestroyRenderResource::operator()(RHI_Object* object) const
    {
        if (object->is_destroyable())
        {
            if (Thread::this_thread() == engine_instance->thread(ThreadType::RenderThread))
            {
                engine_instance->rhi()->destroy_object(object);
            }
            else
            {
                engine_instance->thread(ThreadType::RenderThread)->insert_new_task<DestroyRenderResourceTask>(object);
            }
        }
    }

    void RenderResource::release_render_resouce(RHI_Object* object)
    {
        DestroyRenderResource()(object);
    }

    RenderResource::RenderResource()
    {
        _M_rhi_object = nullptr;
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
            render_thread->insert_new_task<InitRenderResourceTask>(this, _M_rhi_object.get() != nullptr);
        }

        return *this;
    }

    RenderResource& RenderResource::rhi_destroy()
    {
        _M_rhi_object = nullptr;
        return *this;
    }

    RenderResource& RenderResource::postload()
    {
        return init_resource();
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
        if (_M_rhi_object)
        {
            rhi_object<RHI_BindingObject>()->bind(location);
        }
        return *this;
    }

    InitRenderResourceTask::InitRenderResourceTask(RenderResource* object, bool wait) : resource(object), wait(wait)
    {}

    int_t InitRenderResourceTask::execute()
    {
        if (resource)
        {
            if (wait)
            {
                engine_instance->rhi()->wait_idle();
            }
            resource->rhi_create();
        }

        return sizeof(InitRenderResourceTask);
    }
}// namespace Engine
