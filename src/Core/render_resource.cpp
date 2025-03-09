#include <Core/base_engine.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/class.hpp>
#include <Core/render_resource.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(RenderResource, 0);

	void RenderResourcePtrBase::release(void* object)
	{
		RHI_Object::static_release(static_cast<RHI_Object*>(object));
	}

	RenderResource& RenderResource::init_render_resources()
	{
		return *this;
	}

	RenderResource& RenderResource::release_render_resources()
	{
		return *this;
	}

	RenderResource& RenderResource::postload()
	{
		Super::postload();
		return init_render_resources();
	}

	RenderResource& RenderResource::on_destroy()
	{
		Super::on_destroy();
		release_render_resources();
		return *this;
	}
}// namespace Engine
