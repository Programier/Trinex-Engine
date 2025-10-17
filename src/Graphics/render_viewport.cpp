#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <RHI/rhi.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Window/window.hpp>

namespace Engine
{
	static ScriptFunction vc_update;
	static ScriptFunction vc_on_bind_viewport;
	static ScriptFunction vc_on_unbind_viewport;
	static ScriptFunction vc_render;

	trinex_implement_engine_class(RenderViewport, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());
		// r.method("Vector2f size() const final", &This::size);
		// r.method("RenderViewport vsync(bool) final", method_of<RenderViewport&>(&This::vsync));
		// r.method("ViewportClient client() const final", method_of<ViewportClient*>(&This::client));
		// r.method("RenderViewport client(ViewportClient) final", method_of<RenderViewport&>(&This::client));
	}

	trinex_implement_engine_class_default_init(WindowRenderViewport, 0);
	trinex_implement_engine_class_default_init(SurfaceRenderViewport, 0);

	trinex_implement_engine_class(ViewportClient, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());

		vc_update           = r.method("void update(RenderViewport viewport, float dt)", trinex_scoped_method(This, update));
		vc_on_bind_viewport = r.method("void on_bind_viewport(RenderViewport)", trinex_scoped_method(This, on_bind_viewport));
		vc_on_unbind_viewport =
		        r.method("void on_unbind_viewport(RenderViewport)", trinex_scoped_method(This, on_unbind_viewport));

		// Need to check, can we use script engine in multi-thread mode?
		//vc_render = r.method("void render(RenderViewport viewport)", trinex_scoped_method(This, render));

		ScriptEngine::on_terminate.push([]() {
			vc_update.release();
			vc_on_bind_viewport.release();
			vc_on_unbind_viewport.release();
			vc_render.release();
		});
	}

	static RenderViewport* m_current_render_viewport = nullptr;

	ViewportClient& ViewportClient::on_bind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::render(RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::update(class RenderViewport* viewport, float dt)
	{
		return *this;
	}

	ViewportClient* ViewportClient::create(const StringView& name)
	{
		auto* client_class = Refl::Class::static_find(name);

		if (client_class)
		{
			return Object::instance_cast<ViewportClient>(client_class->create_object());
		}

		return nullptr;
	}

	Vector<RenderViewport*> RenderViewport::m_viewports;

	RenderViewport::RenderViewport()
	{
		m_viewports.push_back(this);
	}

	RenderViewport::~RenderViewport()
	{
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

			if (tmp->class_instance()->is_native())
			{
				tmp->on_unbind_viewport(this);
			}
			else
			{
				ScriptObject obj(tmp);
				obj.execute(vc_on_unbind_viewport, this);
			}
		}

		m_client = new_client;

		if (new_client)
		{
			if (new_client->class_instance()->is_native())
			{
				new_client->on_bind_viewport(this);
			}
			else
			{
				ScriptObject obj(new_client);
				obj.execute(vc_on_bind_viewport, this);
			}
		}
		return *this;
	}

	RenderViewport& RenderViewport::update(float dt)
	{
		if (m_client)
		{
			m_current_render_viewport = this;

			if (m_client->class_instance()->is_native())
			{
				m_client->update(this, dt);
			}
			else
			{
				ScriptObject obj(m_client);
				obj.execute(vc_update, this, dt);
			}
			m_current_render_viewport = nullptr;
		}
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

	WindowRenderViewport::WindowRenderViewport(Window* window, bool vsync)
	{
		m_window = window;
		render_thread()->call([vp = Pointer(this), vsync]() { vp->m_swapchain = rhi->create_swapchain(vp->window(), vsync); });
		m_size = window->size();
	}

	WindowRenderViewport::~WindowRenderViewport()
	{
		client(nullptr);
		m_window->m_render_viewport = nullptr;
		m_window                    = nullptr;

		render_thread()->call([swapchain = m_swapchain]() { swapchain->release(); });
		m_swapchain = nullptr;
	}

	Window* WindowRenderViewport::window() const
	{
		return m_window;
	}

	WindowRenderViewport& WindowRenderViewport::rhi_present()
	{
		rhi->present(m_swapchain);
		return *this;
	}

	RHIRenderTargetView* WindowRenderViewport::rhi_rtv()
	{
		return m_swapchain->as_rtv();
	}

	RHITexture* WindowRenderViewport::rhi_texture()
	{
		return m_swapchain->as_texture();
	}

	WindowRenderViewport& WindowRenderViewport::vsync(bool flag)
	{
		if (is_in_render_thread())
		{
			m_swapchain->vsync(flag);
		}
		else
		{
			render_thread()->call([swapchain = m_swapchain, flag]() { swapchain->vsync(flag); });
		}
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::on_resize(const Vector2u& size)
	{
		if (is_in_render_thread())
		{
			m_swapchain->resize(size);
		}
		else
		{
			m_size = size;
			render_thread()->call([swapchain = m_swapchain, size]() { swapchain->resize(size); });
		}
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::on_orientation_changed(Orientation orientation)
	{
		if (is_in_render_thread())
		{
			m_swapchain->resize(size());
		}
		else
		{
			render_thread()->call([self = Pointer(this), orientation]() { self->on_orientation_changed(orientation); });
		}
		return *this;
	}

	SurfaceRenderViewport::SurfaceRenderViewport(RenderSurface* surface)
	{
		m_surface = surface;
		if (surface)
			m_size = surface->size();
	}

	RenderSurface* SurfaceRenderViewport::render_surface() const
	{
		return m_surface;
	}

	SurfaceRenderViewport& SurfaceRenderViewport::rhi_present()
	{
		return *this;
	}
}// namespace Engine
