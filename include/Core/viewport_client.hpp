#pragma once
#include <Core/deprecated/object.hpp>
#include <Input/types.hpp>

namespace Trinex
{
	class trinex_api ViewportClient : public DeprecatedObject
	{
		trinex_class(ViewportClient, DeprecatedObject);

	public:
		virtual ViewportClient& attach(class RenderViewport* viewport);
		virtual ViewportClient& deattach(class RenderViewport* viewport);

		// Return true to consume the event.
		virtual bool on_input_event(const InputEvent& event);

		virtual ViewportClient& update(class RenderViewport* viewport, float dt);
		static ViewportClient* create(const StringView& name);
	};
} // namespace Trinex
