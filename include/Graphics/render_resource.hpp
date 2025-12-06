#pragma once
#include <Core/object.hpp>
#include <RHI/resource_ptr.hpp>

namespace Engine
{
	class ENGINE_EXPORT RenderResource : public Object
	{
		trinex_class(RenderResource, Object);

	public:
		virtual RenderResource& init_render_resources();
		virtual RenderResource& release_render_resources();

		RenderResource& on_destroy() override;
		RenderResource& postload() override;
	};
}// namespace Engine
