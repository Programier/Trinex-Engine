#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
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
		auto r = ScriptClassRegistrar::existing_class(static_class_instance());
		// r.method("Vector2f size() const final", &This::size);
		// r.method("RenderViewport vsync(bool) final", method_of<RenderViewport&>(&This::vsync));
		// r.method("ViewportClient client() const final", method_of<ViewportClient*>(&This::client));
		// r.method("RenderViewport client(ViewportClient) final", method_of<RenderViewport&>(&This::client));
	}

	trinex_implement_engine_class_default_init(WindowRenderViewport, 0);
	trinex_implement_engine_class_default_init(SurfaceRenderViewport, 0);

	trinex_implement_engine_class(ViewportClient, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_class_instance());

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

	static FORCE_INLINE ViewPort construct_default_viewport(Size2D size)
	{
		ViewPort viewport;
		viewport.pos       = {0.f, 0.f};
		viewport.size      = size;
		viewport.min_depth = 0.f;
		viewport.max_depth = 1.f;
		return viewport;
	}

	static FORCE_INLINE Scissor construct_default_scissor(Size2D size)
	{
		Scissor scissor;
		scissor.pos  = {0.f, 0.f};
		scissor.size = size;
		return scissor;
	}

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

	ViewPort ViewportClient::viewport_info(Size2D size) const
	{
		return construct_default_viewport(size);
	}

	Scissor ViewportClient::scissor_info(Size2D size) const
	{
		return construct_default_scissor(size);
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

	ViewPort RenderViewport::viewport_info(Size2D size) const
	{
		if (m_client)
			return m_client->viewport_info(size);
		return construct_default_viewport(size);
	}

	Scissor RenderViewport::scissor_info(Size2D size) const
	{
		if (m_client)
			return m_client->scissor_info(size);
		return construct_default_scissor(size);
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
		if (m_client && !disable_update)
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

	Size2D RenderViewport::size() const
	{
		return m_size;
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
		render_thread()->call([vp = Pointer(this), vsync]() { vp->m_viewport = rhi->create_viewport(vp, vsync); });
	}

	WindowRenderViewport::~WindowRenderViewport()
	{
		client(nullptr);
		m_window->m_render_viewport = nullptr;
		m_window                    = nullptr;

		render_thread()->call([vp = m_viewport]() { vp->release(); });
		m_viewport = nullptr;
	}

	Window* WindowRenderViewport::window() const
	{
		return m_window;
	}

	Size2D WindowRenderViewport::size() const
	{
		if (is_in_render_thread())
			return Super::size();

		return m_window->size() * Settings::screen_percentage;
	}


	WindowRenderViewport& WindowRenderViewport::render()
	{
		if (m_client == nullptr || disable_render)
			return *this;

		class StartRenderingViewport : public Task<StartRenderingViewport>
		{
		public:
			Size2D m_size;
			WindowRenderViewport* m_viewport;

			StartRenderingViewport(WindowRenderViewport* viewport) : m_size(viewport->size()), m_viewport(viewport) {}

			void execute() override
			{
				m_viewport->m_size = m_size;
				rhi->viewport(m_viewport->viewport_info(m_size));
				rhi->scissor(m_viewport->scissor_info(m_size));

				m_viewport->m_client->render(m_viewport);
				m_viewport->m_viewport->present();
			}
		};

		render_thread()->create_task<StartRenderingViewport>(this);
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::rhi_blit_target(RenderSurface* surface, const Rect2D& src, const Rect2D& dst,
																SamplerFilter filter)
	{
		m_viewport->blit_target(surface->rhi_render_target_view(), src, dst, filter);
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::rhi_clear_color(const Color& color)
	{
		m_viewport->clear_color(color);
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::rhi_bind()
	{
		m_viewport->bind();
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::rhi_present()
	{
		m_viewport->present();
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::vsync(bool flag)
	{
		if (is_in_render_thread())
		{
			m_viewport->vsync(flag);
		}
		else
		{
			render_thread()->call([vp = m_viewport, flag]() { vp->vsync(flag); });
		}
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::on_resize(const Size2D& new_size)
	{
		if (is_in_render_thread())
		{
			m_viewport->on_resize(new_size);
		}
		else
		{
			render_thread()->call([vp = m_viewport, new_size]() { vp->on_resize(new_size); });
		}
		return *this;
	}

	WindowRenderViewport& WindowRenderViewport::on_orientation_changed(Orientation orientation)
	{
		if (is_in_render_thread())
		{
			m_viewport->on_orientation_changed(orientation);
		}
		else
		{
			render_thread()->call([vp = m_viewport, orientation]() { vp->on_orientation_changed(orientation); });
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

	Size2D SurfaceRenderViewport::size() const
	{
		if (is_in_render_thread())
			return Super::size();

		return m_surface->size();
	}

	SurfaceRenderViewport& SurfaceRenderViewport::render()
	{
		if (m_client == nullptr || disable_render)
			return *this;

		class StartRenderingViewport : public Task<StartRenderingViewport>
		{
		public:
			Size2D m_size;
			ViewportClient* m_client;
			SurfaceRenderViewport* m_viewport;

			StartRenderingViewport(ViewportClient* client, SurfaceRenderViewport* viewport, Size2D size)
				: m_size(size), m_client(client), m_viewport(viewport)
			{}

			void execute() override
			{
				m_viewport->m_size = m_size;

				rhi->viewport(m_viewport->viewport_info(m_size));
				rhi->scissor(m_viewport->scissor_info(m_size));
				m_client->render(m_viewport);
			}
		};

		render_thread()->create_task<StartRenderingViewport>(m_client.ptr(), this, size());
		return *this;
	}
}// namespace Engine
