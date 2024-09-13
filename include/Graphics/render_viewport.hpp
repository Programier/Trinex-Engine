#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>

namespace Engine
{

	class Window;
	class RenderTarget;
	class RenderSurface;


	class ENGINE_EXPORT ViewportClient : public Object
	{
		declare_class(ViewportClient, Object);

	public:
		virtual ViewportClient& on_bind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& on_unbind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& render(class RenderViewport* viewport);
		virtual ViewportClient& update(class RenderViewport* viewport, float dt);

		static ViewportClient* create(const StringView& name);
	};

	class ENGINE_EXPORT RenderViewport : public RenderResource
	{
		declare_class(RenderViewport, RenderResource);

	private:
		Pointer<ViewportClient> m_client;
		bool m_is_active = true;

	protected:
		static Vector<RenderViewport*> m_viewports;

	public:
		RenderViewport();
		~RenderViewport();

		virtual Window* window() const;
		virtual RenderSurface* render_surface() const;
		virtual Size2D size() const;
		virtual bool is_active() const;

		RenderViewport& is_active(bool active);
		RenderViewport& vsync(bool flag);
		RenderViewport& on_resize(const Size2D& new_size);
		RenderViewport& on_orientation_changed(Orientation orientation);
		RenderViewport& render();

		ViewportClient* client() const;
		RenderViewport& client(ViewportClient* client);
		RenderViewport& update(float dt);

		RenderViewport& rhi_bind();
		RenderViewport& rhi_begin_render();
		RenderViewport& rhi_end_render();
		RenderViewport& rhi_blit_target(RenderSurface* surface, const Rect2D& src, const Rect2D& dst,
		                                SamplerFilter filter = SamplerFilter::Trilinear);
		RenderViewport& rhi_clear_color(const Color& color);

		static RenderViewport* current();
		static const Vector<RenderViewport*>& viewports();
	};

	class ENGINE_EXPORT WindowRenderViewport : public RenderViewport
	{
		declare_class(WindowRenderViewport, RenderViewport);

		bool m_vsync = true;
		class Window* m_window;

	public:
		WindowRenderViewport(Window* window, bool vsync);
		~WindowRenderViewport();
		Window* window() const override;
		Size2D size() const override;
		WindowRenderViewport& rhi_create() override;
	};

	class ENGINE_EXPORT SurfaceRenderViewport : public RenderViewport
	{
		declare_class(SurfaceRenderViewport, RenderViewport);
		Pointer<RenderSurface> m_surface;

	public:
		SurfaceRenderViewport(RenderSurface* surface);
		~SurfaceRenderViewport();
		RenderSurface* render_surface() const override;
		Size2D size() const override;
		SurfaceRenderViewport& rhi_create() override;

		static SurfaceRenderViewport* dummy();
	};
}// namespace Engine
