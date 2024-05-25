#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/exception.hpp>
#include <Core/render_resource.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderResource, Engine, 0);
    implement_default_initialize_class(RenderResource);
    implement_class(BindedRenderResource, Engine, 0);
    implement_default_initialize_class(BindedRenderResource);

    struct DestroyRenderResourceTask : public ExecutableObject {
        RHI_Object* object;

        DestroyRenderResourceTask(RHI_Object* obj) : object(obj)
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
            if (is_in_render_thread())
            {
                rhi->destroy_object(object);
            }
            else
            {
                render_thread()->insert_new_task<DestroyRenderResourceTask>(object);
            }
        }
    }

    void RenderResource::release_render_resouce(RHI_Object* object)
    {
        DestroyRenderResource()(object);
    }

    RenderResource::RenderResource()
    {
        m_rhi_object = nullptr;
    }

    RenderResource& RenderResource::rhi_create()
    {
        return *this;
    }

    RenderResource& RenderResource::init_resource(bool wait_initialize)
    {
        if (is_in_render_thread())
        {
            rhi_create();
        }
        else
        {
            render_thread()->insert_new_task<InitRenderResourceTask>(this, m_rhi_object.get() != nullptr);

            if (wait_initialize)
            {
                render_thread()->wait_all();
            }
        }

        return *this;
    }

    RenderResource& RenderResource::rhi_destroy()
    {
        m_rhi_object = nullptr;
        return *this;
    }

    RenderResource& RenderResource::postload()
    {
        Super::postload();
        return init_resource();
    }

    bool RenderResource::has_object() const
    {
        return m_rhi_object != nullptr;
    }

    RenderResource::~RenderResource()
    {
        rhi_destroy();
    }

    const BindedRenderResource& BindedRenderResource::rhi_bind(BindLocation location) const
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_BindingObject>()->bind(location);
        }
        return *this;
    }

    InitRenderResourceTask::InitRenderResourceTask(RenderResource* object, bool wait) : resource(object), wait(wait)
    {
        if (object == nullptr)
        {
            throw EngineException("Cannot init render resource! Resource is null!");
        }
    }

    int_t InitRenderResourceTask::execute()
    {
        if (wait)
        {
            rhi->wait_idle();
        }
        resource->rhi_create();

        return sizeof(InitRenderResourceTask);
    }
}// namespace Engine
