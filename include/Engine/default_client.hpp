#pragma once
#include <Graphics/render_viewport.hpp>

namespace Engine
{
	// This viewport can be used for testing something
	class ENGINE_EXPORT DefaultClient : public ViewportClient
	{
		trinex_declare_class(DefaultClient, ViewportClient);

	public:
		DefaultClient();
		~DefaultClient();
		DefaultClient& on_bind_viewport(class RenderViewport* viewport) override;
		DefaultClient& update(class RenderViewport* viewport, float dt) override;
	};
}// namespace Engine
