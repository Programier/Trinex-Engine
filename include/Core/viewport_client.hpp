#pragma once
#include <Core/object.hpp>
#include <Input/types.hpp>

namespace Trinex
{
	class ENGINE_EXPORT ViewportClient : public Object
	{
		trinex_class(ViewportClient, Object);

	public:
		virtual ViewportClient& attach(class RenderViewport* viewport);
		virtual ViewportClient& deattach(class RenderViewport* viewport);

		// Return true to consume the event.
		virtual bool on_input_event(const InputEvent& event);

		virtual ViewportClient& update(class RenderViewport* viewport, float dt);
		static ViewportClient* create(const StringView& name);
	};
}// namespace Trinex
