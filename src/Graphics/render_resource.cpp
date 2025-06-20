#include <Core/reflection/class.hpp>
#include <Graphics/render_resource.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(RenderResource, 0);

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
