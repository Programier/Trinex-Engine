#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/rhi.hpp>
#include <Platform/thread.hpp>

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
            SingleTimeInitRenderResourceTask* task = new SingleTimeInitRenderResourceTask();
            task->object = this;
            render_thread->push_task(task);
        }

        return *this;
    }


    struct DestroyRenderResource : public SingleTimeExecutableObject {
        RHI_Object* object;
        int_t execute()
        {
            delete object;
            return 0;
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
                DestroyRenderResource* task = new DestroyRenderResource();
                task->object                = _M_rhi_object;
                engine_instance->thread(ThreadType::RenderThread)->push_task(task);
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

    int_t InitRenderResourceTask::execute()
    {
        if (object)
        {
            object->rhi_create();
        }

        return ExecutableObject::execute();
    }

    int_t SingleTimeInitRenderResourceTask::execute()
    {
        if (object)
        {
            object->rhi_create();
        }

        return SingleTimeExecutableObject::execute();
    }
}// namespace Engine
