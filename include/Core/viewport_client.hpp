#pragma once
#include <Core/object.hpp>

namespace Trinex
{
	class ENGINE_EXPORT ViewportClient : public Object
	{
		trinex_class(ViewportClient, Object);

	public:
		virtual ViewportClient& attach(class RenderViewport* viewport);
		virtual ViewportClient& deattach(class RenderViewport* viewport);

		virtual ViewportClient& update(class RenderViewport* viewport, float dt);
		static ViewportClient* create(const StringView& name);
	};
}// namespace Trinex
