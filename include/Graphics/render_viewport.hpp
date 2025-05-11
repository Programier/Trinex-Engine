#pragma once
#include <Core/etl/string.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	class Window;
	class RenderTarget;
	class RenderSurface;
	struct RHI_RenderTargetView;
	struct Color;

	class ENGINE_EXPORT ViewportClient : public Object
	{
		trinex_declare_class(ViewportClient, Object);

	public:
		virtual ViewportClient& on_bind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& on_unbind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& render(class RenderViewport* viewport);
		virtual ViewportClient& update(class RenderViewport* viewport, float dt);

		virtual ViewPort viewport_info(Size2D size) const;
		virtual Scissor scissor_info(Size2D size) const;

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

		virtual RenderViewport& rhi_blit_target(RHI_RenderTargetView* surface, const Rect2D& src, const Rect2D& dst,
		                                        SamplerFilter filter = SamplerFilter::Trilinear) = 0;
		virtual RenderViewport& rhi_clear_color(const Color& color)                              = 0;
		virtual RenderViewport& rhi_bind()                                                       = 0;
		virtual RenderViewport& rhi_present()                                                    = 0;
		inline Size2D size() const { return m_size; }

		RenderViewport& update(float dt);
		ViewportClient* client() const;
		RenderViewport& client(ViewportClient* client);

		ViewPort viewport_info(Size2D size) const;
		Scissor scissor_info(Size2D size) const;

		FORCE_INLINE ViewPort viewport_info() const { return viewport_info(size()); }

		FORCE_INLINE Scissor scissor_info() const { return scissor_info(size()); }

		static RenderViewport* current();
		static const Vector<RenderViewport*>& viewports();
	};

	class ENGINE_EXPORT WindowRenderViewport : public RenderViewport
	{
		trinex_declare_class(WindowRenderViewport, RenderViewport);
		class Window* m_window;
		struct RHI_Viewport* m_viewport;

	public:
		WindowRenderViewport(Window* window, bool vsync);
		~WindowRenderViewport();
		Window* window() const;
		WindowRenderViewport& rhi_blit_target(RHI_RenderTargetView* surface, const Rect2D& src, const Rect2D& dst,
		                                      SamplerFilter filter = SamplerFilter::Trilinear) override;
		WindowRenderViewport& rhi_clear_color(const Color& color) override;
		WindowRenderViewport& rhi_bind() override;
		WindowRenderViewport& rhi_present() override;

		WindowRenderViewport& vsync(bool flag);
		WindowRenderViewport& on_resize(const Size2D& new_size);
		WindowRenderViewport& on_orientation_changed(Orientation orientation);
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
