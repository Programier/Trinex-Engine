#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
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

	implement_engine_class(RenderViewport, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			r->method("Vector2D size() const final", &This::size);
			r->method("RenderViewport vsync(bool) final", method_of<RenderViewport&>(&This::vsync));
			r->method("ViewportClient client() const final", method_of<ViewportClient*>(&This::client));
			r->method("RenderViewport client(ViewportClient) final", method_of<RenderViewport&>(&This::client));
		};
	}

	implement_engine_class_default_init(WindowRenderViewport, 0);
	implement_engine_class_default_init(SurfaceRenderViewport, 0);

	implement_engine_class(ViewportClient, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			vc_update             = r->method("void update(RenderViewport viewport, float dt)", &This::update);
			vc_on_bind_viewport   = r->method("void on_bind_viewport(RenderViewport)", &This::on_bind_viewport);
			vc_on_unbind_viewport = r->method("void on_unbind_viewport(RenderViewport)", &This::on_unbind_viewport);

			// Need to check, can we use script engine in multi-thread mode?
			// vc_render = r->method("void render(RenderViewport viewport)", &This::render);
		};

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
		Class* client_class = Class::static_find(name);

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

	Window* RenderViewport::window() const
	{
		return nullptr;
	}

	RenderSurface* RenderViewport::render_surface() const
	{
		return nullptr;
	}

	RenderViewport& RenderViewport::vsync(bool flag)
	{

		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			call_in_render_thread([viewport, flag]() { viewport->vsync(flag); });
		}

		return *this;
	}

	RenderViewport& RenderViewport::on_resize(const Size2D& new_size)
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			call_in_render_thread([viewport, new_size]() { viewport->on_resize(new_size); });
		}
		return *this;
	}

	RenderViewport& RenderViewport::on_orientation_changed(Orientation orientation)
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			call_in_render_thread([viewport, orientation]() { viewport->on_orientation_changed(orientation); });
		}
		return *this;
	}


	Size2D RenderViewport::size() const
	{
		return {};
	}

	bool RenderViewport::is_active() const
	{
		return m_is_active;
	}

	RenderViewport& RenderViewport::is_active(bool active)
	{
		m_is_active = active;
		return *this;
	}

	class StartRenderingViewport : public ExecutableObject
	{
	public:
		ViewportClient* m_client;
		RenderViewport* m_viewport;
		RHI_Viewport* m_rhi_viewport;

		StartRenderingViewport(ViewportClient* client, RenderViewport* viewport, RHI_Viewport* rhi_viewport)
		    : m_client(client), m_viewport(viewport), m_rhi_viewport(rhi_viewport)
		{}

		int_t execute() override
		{
			m_rhi_viewport->begin_render();
			if (m_client)
				m_client->render(m_viewport);
			m_rhi_viewport->end_render();

			return sizeof(StartRenderingViewport);
		}
	};

	RenderViewport& RenderViewport::render()
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport == nullptr)
			return *this;

		SceneRenderTargets::instance()->initialize(size() * Settings::e_screen_percentage);
		render_thread()->insert_new_task<StartRenderingViewport>(m_client.ptr(), this, viewport);
		return *this;
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
				obj.execute(vc_on_unbind_viewport, ScriptObject(this));
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
				obj.execute(vc_on_bind_viewport, ScriptObject(this));
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
				obj.execute(vc_update, ScriptObject(this), dt);
			}
			m_current_render_viewport = nullptr;
		}
		return *this;
	}

	RenderViewport& RenderViewport::rhi_bind()
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			viewport->bind();
		}
		return *this;
	}

	RenderViewport& RenderViewport::rhi_begin_render()
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			viewport->begin_render();
		}
		return *this;
	}

	RenderViewport& RenderViewport::rhi_end_render()
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			viewport->end_render();
		}
		return *this;
	}

	RenderViewport& RenderViewport::rhi_blit_target(RenderSurface* surface, const Rect2D& src, const Rect2D& dst,
	                                                SamplerFilter filter)
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			viewport->blit_target(surface, src, dst, filter);
		}
		return *this;
	}

	RenderViewport& RenderViewport::rhi_clear_color(const Color& color)
	{
		RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
		if (viewport)
		{
			viewport->clear_color(color);
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
		m_vsync  = vsync;
	}

	WindowRenderViewport::~WindowRenderViewport()
	{
		if (m_window)
		{
			m_window->m_render_viewport = nullptr;
			m_window                    = nullptr;
		}
	}

	Window* WindowRenderViewport::window() const
	{
		return m_window;
	}

	Size2D WindowRenderViewport::size() const
	{
		return m_window->size();
	}

	WindowRenderViewport& WindowRenderViewport::rhi_create()
	{
		m_rhi_object.reset(rhi->create_viewport(this, m_vsync));
		return *this;
	}

	SurfaceRenderViewport::SurfaceRenderViewport(RenderSurface* surface)
	{
		m_surface = surface;
	}

	SurfaceRenderViewport::~SurfaceRenderViewport()
	{}

	RenderSurface* SurfaceRenderViewport::render_surface() const
	{
		return m_surface;
	}

	Size2D SurfaceRenderViewport::size() const
	{
		return m_surface->size();
	}

	SurfaceRenderViewport& SurfaceRenderViewport::rhi_create()
	{
		m_rhi_object.reset(rhi->create_viewport(this));
		return *this;
	}


	class DummySurfaceRenderViewport : public SurfaceRenderViewport
	{
	public:
		DummySurfaceRenderViewport() : SurfaceRenderViewport(nullptr)
		{
			m_viewports.pop_back(); // Unregister this viewport from viewports array
		}

		bool is_active() const override
		{
			return false;
		}
	};

	SurfaceRenderViewport* SurfaceRenderViewport::dummy()
	{
		static SurfaceRenderViewport* dummy_viewport = nullptr;

		if (dummy_viewport == nullptr)
		{
			dummy_viewport = Object::new_instance<EngineResource<DummySurfaceRenderViewport>>();
			dummy_viewport->init_resource();
		}
		return dummy_viewport;
	}
}// namespace Engine
