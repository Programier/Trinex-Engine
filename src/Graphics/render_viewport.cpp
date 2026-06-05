#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/viewport_client.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/rhi.hpp>
#include <Window/window.hpp>

namespace Trinex
{
	trinex_implement_engine_class(RenderViewport, Refl::Class::IsScriptable) {}

	static RenderViewport* m_current_render_viewport = nullptr;
	Vector<RenderViewport*> RenderViewport::m_viewports;

	RenderViewport::RenderViewport(Window* window, bool vsync)
	{
		m_viewports.push_back(this);
		m_window    = window;
		m_swapchain = RHI::instance()->create_swapchain(window, vsync);
		m_size      = window->size();
	}

	RenderViewport::~RenderViewport()
	{
		client(nullptr);
		m_window->m_render_viewport = nullptr;
		m_window                    = nullptr;
		m_swapchain                 = nullptr;

		auto it = std::remove_if(m_viewports.begin(), m_viewports.end(), [this](RenderViewport* vp) { return vp == this; });
		m_viewports.erase(it, m_viewports.end());
	}

	ViewportClient* RenderViewport::client() const
	{
		return m_client.ptr();
	}

	RenderViewport& RenderViewport::client(ViewportClient* new_client)
	{
		if (m_client)
		{
			ViewportClient* tmp = m_client;
			m_client            = nullptr;
			tmp->deattach(this);
		}

		m_client = new_client;

		if (new_client)
		{
			new_client->attach(this);
		}
		return *this;
	}

	RenderViewport& RenderViewport::update(float dt)
	{
		if (m_client)
		{
			m_current_render_viewport = this;
			m_client->update(this, dt);
			m_current_render_viewport = nullptr;
		}
		return *this;
	}

	RenderViewport& RenderViewport::vsync(bool flag)
	{
		m_swapchain->vsync(flag);
		return *this;
	}

	RenderViewport& RenderViewport::on_resize(const Vector2u& size)
	{
		m_size = size;
		m_swapchain->resize(size);
		return *this;
	}

	RenderViewport& RenderViewport::on_orientation_changed(Orientation orientation)
	{
		m_swapchain->resize(size());
		return *this;
	}

	RenderViewport* RenderViewport::current()
	{
		return m_current_render_viewport;
	}

	const Vector<RenderViewport*>& RenderViewport::viewports()
	{
		return m_viewports;
	}
}// namespace Trinex
