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

    struct DestroyRenderResourceTask : public ExecutableObject {
        RHI_Object* object;

        DestroyRenderResourceTask(RHI_Object* obj) : object(obj)
        {}

        int_t execute()
        {
            delete object;
            return sizeof(DestroyRenderResourceTask);
        }
    };


    void RenderResource::DestroyRenderResource::operator()(RHI_Object* object) const
    {
        if (object->is_destroyable())
        {
            if (Thread::this_thread() == engine_instance->thread(ThreadType::RenderThread))
            {
                delete object;
            }
            else
            {
                engine_instance->thread(ThreadType::RenderThread)->insert_new_task<DestroyRenderResourceTask>(object);
            }
        }
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
            render_thread->insert_new_task<InitRenderResourceTask>(this);
        }

        return *this;
    }


    RenderResource& RenderResource::rhi_destroy()
    {
        _M_rhi_object = nullptr;
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
        if (_M_rhi_object)
        {
            rhi_object<RHI_BindingObject>()->bind(location);
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
