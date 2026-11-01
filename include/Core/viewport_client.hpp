#pragma once
#include <Core/object.hpp>
#include <Input/types.hpp>

#include <Core/viewport_client.generated.hpp>

namespace Trinex
{
	class RenderGraph;
	struct InputEvent;

	trinex_reflect()
	class trinex_api ViewportClient : public Object
	{
		trinex_reflection_body();

	public:
		virtual ViewportClient& attach(class RenderViewport* viewport);
		virtual ViewportClient& deattach(class RenderViewport* viewport);

		// Return true to consume the event.
		virtual bool on_input_event(const InputEvent& event);

		virtual void update(class RenderViewport* viewport, f32 dt);
		virtual void render(class RenderViewport* viewport, RenderGraph* graph);
		static Ref<ViewportClient> create(StringView name);
	};
} // namespace Trinex
