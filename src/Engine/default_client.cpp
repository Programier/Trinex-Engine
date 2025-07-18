#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/types/color.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/pipeline.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	DefaultClient::DefaultClient() {}

	DefaultClient::~DefaultClient() {}

	DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
	{
		render_thread()->call([viewport]() {
			rhi->submit();
			auto rtv = viewport->rhi_rtv();
			rtv->clear(LinearColor(1, 0, 0, 1));
			viewport->rhi_present();
		});
		return *this;
	}

	trinex_implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
