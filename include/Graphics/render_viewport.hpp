#pragma once
#include <Core/etl/string.hpp>
#include <Core/math/vector.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <RHI/enums.hpp>
#include <RHI/resource_ptr.hpp>

namespace Trinex
{
	class Window;
	class RenderTarget;
	class RenderSurface;
	class RHIRenderTargetView;
	class RHITexture;
	struct Color;

	class ENGINE_EXPORT ViewportClient : public Object
	{
		trinex_class(ViewportClient, Object);

	public:
		virtual ViewportClient& on_bind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& on_unbind_viewport(class RenderViewport* viewport);
		virtual ViewportClient& render(class RenderViewport* viewport);
		virtual ViewportClient& update(class RenderViewport* viewport, float dt);
		static ViewportClient* create(const StringView& name);
	};

	class ENGINE_EXPORT RenderViewport : public Object
	{
		trinex_class(RenderViewport, Object);

	private:
		static Vector<RenderViewport*> m_viewports;

		class Window* m_window;
		RHIResourcePtr<class RHISwapchain> m_swapchain;
		Pointer<ViewportClient> m_client;
		Vector2u m_size;

	public:
		RenderViewport(Window* window, bool vsync);
		~RenderViewport();

		RenderViewport& update(float dt);
		ViewportClient* client() const;
		RenderViewport& client(ViewportClient* client);
		RenderViewport& vsync(bool flag);
		RenderViewport& on_resize(const Vector2u& size);
		RenderViewport& on_orientation_changed(Orientation orientation);

		inline Window* window() const { return m_window; }
		inline RHISwapchain* swapchain() const { return m_swapchain; }
		inline Vector2u size() const { return m_size; }
		inline float aspect() const { return static_cast<float>(m_size.x) / static_cast<float>(m_size.y); }

		static RenderViewport* current();
		static const Vector<RenderViewport*>& viewports();
	};
}// namespace Trinex
