#include <Core/engine.hpp>
#include <Core/viewport_client.hpp>
#include <Core/window.hpp>
#include <Entry/debug_engine.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	static DebugEngineEntry* s_entry = nullptr;

	class DebugViewportClient : public ViewportClient
	{
	public:
		void update(class RenderViewport* viewport, f32 dt) override {}

		void render(class RenderViewport* viewport, RenderGraph* graph) override {}
	};

	void DebugEngineEntry::on_start()
	{
		auto window = Window::create("Debug Engine", {1280, 720});
		m_viewport  = Ref<RenderViewport>::make(etl::move(window), RHIPresentMode::Fifo);
		m_viewport->client(Ref<DebugViewportClient>::make());
	}

	i32 DebugEngineEntry::execute()
	{
		s_entry = this;
		LifeCycle::on_start([] { s_entry->on_start(); });

		return Super::execute();
	}
} // namespace Trinex
