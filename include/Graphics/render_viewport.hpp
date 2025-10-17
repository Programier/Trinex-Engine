#pragma once
#include <Core/etl/string.hpp>
#include <Core/math/vector.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class Window;
	class RenderTarget;
	class RenderSurface;
	class RHIRenderTargetView;
	class RHITexture;
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
		Vector2u m_size;
		static Vector<RenderViewport*> m_viewports;

	public:
		RenderViewport();
		~RenderViewport();

		virtual RHIRenderTargetView* rhi_rtv() = 0;
		virtual RHITexture* rhi_texture()      = 0;
		virtual RenderViewport& rhi_present()  = 0;
		inline Vector2u size() const { return m_size; }
		inline float aspect() const { return static_cast<float>(m_size.x) / static_cast<float>(m_size.y); }

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
		WindowRenderViewport& on_resize(const Vector2u& size);
		WindowRenderViewport& on_orientation_changed(Orientation orientation);

		WindowRenderViewport& rhi_present() override;
		RHIRenderTargetView* rhi_rtv() override;
		RHITexture* rhi_texture() override;
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
