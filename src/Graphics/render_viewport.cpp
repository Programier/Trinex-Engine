#include <Core/base_engine.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/viewport_client.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Input/event_system.hpp>
#include <RHI/rhi.hpp>
#include <Window/window.hpp>

namespace Trinex
{
	trinex_implement_engine_class(RenderViewport, Refl::Class::IsScriptable) {}

	static RenderViewport* m_current_render_viewport = nullptr;
	Vector<RenderViewport*> RenderViewport::m_viewports;

	namespace
	{
		static RenderViewport* find_viewport_by_window_id(Identifier window_id)
		{
			for (RenderViewport* viewport : RenderViewport::viewports())
			{
				if (viewport && viewport->window() && viewport->window()->id() == window_id)
				{
					return viewport;
				}
			}

			return nullptr;
		}

		class ViewportEventBridge final : public EventListener
		{
		public:
			EventDispatchResult on_event(RoutedEvent& event) override
			{
				EventDispatchResult result;

				if (event.header.type_id == EventTypeIds::Quit)
				{
					for (RenderViewport* viewport : RenderViewport::viewports())
					{
						if (viewport && viewport->client())
						{
							EventDispatchResult client_result = viewport->client()->on_event(event);
							result.handled |= client_result.handled;
							result.consumed |= client_result.consumed;
							result.continue_propagation &= client_result.continue_propagation;
							result.emit_gameplay_actions &= client_result.emit_gameplay_actions;

							if (!client_result.continue_propagation)
							{
								break;
							}
						}
					}

					if (result.consumed)
					{
						event.mark_consumed();
					}
					else if (result.handled)
					{
						event.mark_handled();
					}

					event.result = result;
					return result;
				}

				if (event.header.window_id != 0)
				{
					if (RenderViewport* viewport = find_viewport_by_window_id(event.header.window_id))
					{
						if (ViewportClient* client = viewport->client())
						{
							result = client->on_event(event);
						}
					}
				}
				else
				{
					for (RenderViewport* viewport : RenderViewport::viewports())
					{
						if (viewport && viewport->client())
						{
							EventDispatchResult client_result = viewport->client()->on_event(event);
							result.handled |= client_result.handled;
							result.consumed |= client_result.consumed;
							result.continue_propagation &= client_result.continue_propagation;
							result.emit_gameplay_actions &= client_result.emit_gameplay_actions;

							if (!client_result.continue_propagation)
							{
								break;
							}
						}
					}
				}

				if (result.consumed)
				{
					event.mark_consumed();
				}
				else if (result.handled)
				{
					event.mark_handled();
				}

				event.result = result;
				return result;
			}
		};

		static ViewportEventBridge s_viewport_event_bridge;
		static bool s_viewport_event_bridge_registered = false;

		static void register_viewport_event_bridge()
		{
			if (s_viewport_event_bridge_registered)
				return;

			if (EventSystem* system = EventSystem::instance())
			{
				system->dispatcher().add_listener(EventTypeIds::Quit, &s_viewport_event_bridge);
				system->dispatcher().add_listener(EventTypeIds::Window, &s_viewport_event_bridge);
				system->dispatcher().add_listener(EventTypeIds::Key, &s_viewport_event_bridge);
				system->dispatcher().add_listener(EventTypeIds::TextInput, &s_viewport_event_bridge);
				system->dispatcher().add_listener(EventTypeIds::Pointer, &s_viewport_event_bridge);
				system->dispatcher().add_listener(EventTypeIds::Gamepad, &s_viewport_event_bridge);
				system->dispatcher().add_listener(EventTypeIds::DeviceChange, &s_viewport_event_bridge);
				s_viewport_event_bridge_registered = true;
			}
		}
	}// namespace

	RenderViewport::RenderViewport(Window* window, u32 present_interval)
	{
		register_viewport_event_bridge();
		m_viewports.push_back(this);
		m_window    = window;
		m_swapchain = RHI::instance()->create_swapchain(window, present_interval);
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

	RenderViewport& RenderViewport::present_interval(u32 interval)
	{
		m_swapchain->present_interval(interval);
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
