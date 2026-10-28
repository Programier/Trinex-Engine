#include <Core/console.hpp>
#include <Core/engine.hpp>
#include <Core/etl/map.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/reflection/deprecated/class.hpp>
#include <Core/reflection/of.hpp>
#include <Core/threading.hpp>
#include <Core/viewport_client.hpp>
#include <Core/window.hpp>
#include <Graphics/render_viewport.hpp>
#include <Platform/platform.hpp>

namespace Trinex
{
	static WindowDesc s_config;

	trinex_on_bootstrap()
	{
		Console::property("window.title", &s_config.title);
		Console::property("window.client", &s_config.client);
		Console::property("window.size", &s_config.size);
		Console::property("window.pos", &s_config.pos);
		Console::property("window.monitor", &s_config.monitor);
		//static Console::VariableRef attributes(&s_config.attributes, "window.attributes");
	}

	trinex_api const WindowDesc& WindowDesc::from_config()
	{
		return s_config;
	}

	Window* Window::create(const char* title, Vector2u size, Window* parent)
	{
		WindowDesc desc = {
		        .title = title,
		        .size  = size,
		};

		return create(desc, parent);
	}

	Window* Window::create(const WindowDesc& desc, Window* parent)
	{
		Ref<Window> self; //Platform::WindowSystem::instance()->create(desc);

		if (self == nullptr)
			return nullptr;

		const i32 interval      = 1;
		self->m_render_viewport = DeprecatedObject::new_instance<RenderViewport>("", nullptr, self.value(), interval);

		// Initialize client
		//self->icon(load_image_icon());

		if (!desc.client.empty())
		{
			self->create_client(desc.client);
		}
		return self.detach();
	}

	Ref<Window> Window::find(u32 id)
	{
		return Platform::WindowSystem::instance()->find(id);
	}

	RenderViewport* Window::render_viewport() const
	{
		return m_render_viewport;
	}

	// Window::~Window()
	// {
	// 	if (m_render_viewport)
	// 	{
	// 		RenderViewport* viewport = m_render_viewport;
	// 		m_render_viewport        = nullptr;
	// 		viewport->client(nullptr);
	// 		GarbageCollector::destroy(viewport);
	// 	}
	// }

	Window& Window::create_client(const StringView& client_name)
	{
		if (auto vp = render_viewport())
		{
			ViewportClient* client = ViewportClient::create(client_name);

			if (client)
			{
				vp->client(client);
			}
		}
		return *this;
	}
} // namespace Trinex
