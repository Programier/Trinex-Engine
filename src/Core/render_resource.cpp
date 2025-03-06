#include <Core/base_engine.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/class.hpp>
#include <Core/render_resource.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(RenderResource, 0);
	trinex_implement_engine_class_default_init(BindedRenderResource, 0);

	struct DestroyRenderResourceTask : public Task<DestroyRenderResourceTask> {
		RHI_Object* object;

		DestroyRenderResourceTask(RHI_Object* obj) : object(obj)
		{}

		void execute() override
		{
			object->release();
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
			render_thread()->create_task<DestroyRenderResourceTask>(object);
		}
	}

	RenderResource::RenderResource()
	{
		m_rhi_object = nullptr;
	}

	RenderResource& RenderResource::rhi_init()
	{
		return *this;
	}

	RenderResource& RenderResource::init_resource(bool wait_initialize)
	{
		if (is_in_render_thread())
		{
			rhi_init();
		}
		else
		{
			render_thread()->create_task<InitRenderResourceTask>(this);

			if (wait_initialize)
			{
				render_thread()->wait();
			}
		}

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

	void InitRenderResourceTask::execute()
	{
		resource->rhi_init();
	}
}// namespace Engine
