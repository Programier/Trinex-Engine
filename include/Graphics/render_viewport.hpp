#pragma once
#include <Core/etl/array.hpp>
#include <Core/math/vector.hpp>
#include <Core/object.hpp>
#include <RHI/resource_ptr.hpp>

#include <Graphics/render_viewport.generated.hpp>

namespace Trinex
{
	class Window;
	class ViewportClient;
	class RHISemaphore;
	class RHITexture;
	class RHIContext;
	class RHISwapchain;
	class RenderGraph;
	struct RHIPresentMode;
	struct WindowAttribute;

	trinex_reflect()
	class trinex_api RenderViewport : public Object
	{
		trinex_reflection_body();

	private:
		static Array<Ref<RenderViewport>> s_viewports;

		Ref<ViewportClient> m_client;
		Ref<Window> m_window;
		RHIResourcePtr<RHISwapchain> m_swapchain;

	public:
		RenderViewport(Ref<Window> window, RHIPresentMode mode, u32 images = 0);
		~RenderViewport();

		virtual void update(f32 dt);
		virtual void render(RenderGraph* graph);
		virtual void submit();

		RenderViewport& client(Ref<ViewportClient> client);
		RenderViewport& mode(RHIPresentMode mode);
		RHISemaphore* acquire_semaphore();
		RHISemaphore* present_semaphore();
		RHITexture* backbuffer();

		static inline const Array<Ref<RenderViewport>>& viewports() { return s_viewports; }
		inline const Ref<ViewportClient>& client() const { return m_client; }
		inline const Ref<Window>& window() const { return m_window; }
		inline const RHIResourcePtr<RHISwapchain>& swapchain() const { return m_swapchain; }
	};
} // namespace Trinex
