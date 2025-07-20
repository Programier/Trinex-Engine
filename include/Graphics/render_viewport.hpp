#pragma once
#include <Core/etl/string.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class Window;
	class RenderTarget;
	class RenderSurface;
	class RHIRenderTargetView;
	struct Color;

	class ENGINE_EXPORT ViewportClient : public Object
	{
		trinex_declare_class(ViewportClient, Object);

	public:
		virtual ViewportClient& on_bind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& on_unbind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& render(class RenderViewport* viewport);
		virtual ViewportClient& update(class RenderViewport* viewport, float dt);
		static ViewportClient* create(const StringView& name);
	};

	class ENGINE_EXPORT RenderViewport : public Object
	{
		trinex_declare_class(RenderViewport, Object);

	protected:
		Pointer<ViewportClient> m_client;
		Size2D m_size;
		static Vector<RenderViewport*> m_viewports;

	public:
		RenderViewport();
		~RenderViewport();

		virtual RHIRenderTargetView* rhi_rtv() = 0;
		virtual RenderViewport& rhi_present()   = 0;
		inline Size2D size() const { return m_size; }

		RenderViewport& update(float dt);
		ViewportClient* client() const;
		RenderViewport& client(ViewportClient* client);

		static RenderViewport* current();
		static const Vector<RenderViewport*>& viewports();
	};

	class ENGINE_EXPORT WindowRenderViewport : public RenderViewport
	{
		trinex_declare_class(WindowRenderViewport, RenderViewport);
		class Window* m_window;
		class RHISwapchain* m_swapchain;

	public:
		WindowRenderViewport(Window* window, bool vsync);
		~WindowRenderViewport();
		Window* window() const;

		WindowRenderViewport& vsync(bool flag);
		WindowRenderViewport& on_resize(const Size2D& new_size);
		WindowRenderViewport& on_orientation_changed(Orientation orientation);

		WindowRenderViewport& rhi_present() override;
		RHIRenderTargetView* rhi_rtv() override;
		inline RHISwapchain* rhi_swapchain() const { return m_swapchain; }
	};

	class ENGINE_EXPORT SurfaceRenderViewport : public RenderViewport
	{
		trinex_declare_class(SurfaceRenderViewport, RenderViewport);
		Pointer<RenderSurface> m_surface;

	public:
		SurfaceRenderViewport(RenderSurface* surface);
		RenderSurface* render_surface() const;
		SurfaceRenderViewport& rhi_present() override;
	};
}// namespace Engine
