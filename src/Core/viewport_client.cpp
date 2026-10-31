#include <Core/reflection/class.hpp>
#include <Core/viewport_client.hpp>

namespace Trinex
{
	ViewportClient& ViewportClient::attach(class RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::deattach(class RenderViewport* viewport)
	{
		return *this;
	}

	bool ViewportClient::on_input_event(const InputEvent& event)
	{
		return false;
	}

	ViewportClient& ViewportClient::update(class RenderViewport* viewport, float dt)
	{
		return *this;
	}

	Ref<ViewportClient> ViewportClient::create(StringView name)
	{
		auto* client_class = Refl::Class::root()->lookup<Refl::Class>(name);

		if (client_class)
		{
			return adopt_ref(client_class->allocate<ViewportClient>());
		}

		return nullptr;
	}
} // namespace Trinex
