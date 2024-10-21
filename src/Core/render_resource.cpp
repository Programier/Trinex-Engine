#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/exception.hpp>
#include <Core/render_resource.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	implement_engine_class_default_init(RenderResource, 0);
	implement_engine_class_default_init(BindedRenderResource, 0);

	struct DestroyRenderResourceTask : public ExecutableObject {
		RHI_Object* object;

		DestroyRenderResourceTask(RHI_Object* obj) : object(obj)
		{}

		int_t execute()
		{
			object->release();
			return sizeof(DestroyRenderResourceTask);
		}
	};


	void RenderResource::DestroyRenderResource::operator()(RHI_Object* object) const
	{
		if (is_in_render_thread())
		{
			object->release();
		}
		else
		{
			render_thread()->insert_new_task<DestroyRenderResourceTask>(object);
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
			render_thread()->insert_new_task<InitRenderResourceTask>(this);

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

	InitRenderResourceTask::InitRenderResourceTask(RenderResource* object) : resource(object)
	{
		if (object == nullptr)
		{
			throw EngineException("Cannot init render resource! Resource is null!");
		}
	}

	int_t InitRenderResourceTask::execute()
	{
		resource->rhi_create();
		return sizeof(InitRenderResourceTask);
	}
}// namespace Engine
