#include <Input/event_system.hpp>
#include <Core/reflection/class.hpp>
#include <Core/viewport_client.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Trinex
{
	static ScriptFunction vc_update;
	static ScriptFunction vc_attach;
	static ScriptFunction vc_deattach;
	static ScriptFunction vc_render;

	trinex_implement_engine_class(ViewportClient, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());

		vc_update   = r.method("void update(RenderViewport viewport, float dt)", trinex_scoped_method(This, update));
		vc_attach   = r.method("void attach(RenderViewport)", trinex_scoped_method(This, attach));
		vc_deattach = r.method("void deattach(RenderViewport)", trinex_scoped_method(This, deattach));

			// Need to check, can we use script engine in multi-thread mode?
			//vc_render = r.method("void render(RenderViewport viewport)", trinex_scoped_method(This, render));

		ScriptEngine::on_terminate.push([]() {
			vc_update.release();
			vc_attach.release();
			vc_deattach.release();
			vc_render.release();
		});
	}

	ViewportClient& ViewportClient::attach(class RenderViewport* viewport)
	{
		return *this;
	}

	ViewportClient& ViewportClient::deattach(class RenderViewport* viewport)
	{
		return *this;
	}

	EventDispatchResult ViewportClient::on_event(RoutedEvent& event)
	{
		switch (event.header.type_id)
		{
			case EventTypeIds::Quit: return on_quit(event);

			case EventTypeIds::Window:
			{
				auto* payload = reinterpret_cast<WindowEvent*>(event.payload);
				if (payload)
					return on_window_event(*payload);
				break;
			}

			case EventTypeIds::Key:
			{
				auto* payload = reinterpret_cast<KeyEvent*>(event.payload);
				if (payload)
					return on_key_event(*payload);
				break;
			}

			case EventTypeIds::TextInput:
			{
				auto* payload = reinterpret_cast<TextInputEvent*>(event.payload);
				if (payload)
					return on_text_input_event(*payload);
				break;
			}

			case EventTypeIds::Pointer:
			{
				auto* payload = reinterpret_cast<PointerEvent*>(event.payload);
				if (payload)
					return on_pointer_event(*payload);
				break;
			}

			case EventTypeIds::Gamepad:
			{
				auto* payload = reinterpret_cast<GamepadEvent*>(event.payload);
				if (payload)
					return on_gamepad_event(*payload);
				break;
			}

			case EventTypeIds::DeviceChange:
			{
				auto* payload = reinterpret_cast<DeviceChangeEvent*>(event.payload);
				if (payload)
					return on_device_change_event(*payload);
				break;
			}

			default: break;
		}

		return {};
	}

	EventDispatchResult ViewportClient::on_quit(RoutedEvent& event)
	{
		return {};
	}

	EventDispatchResult ViewportClient::on_window_event(WindowEvent& event)
	{
		return {};
	}

	EventDispatchResult ViewportClient::on_key_event(KeyEvent& event)
	{
		return {};
	}

	EventDispatchResult ViewportClient::on_text_input_event(TextInputEvent& event)
	{
		return {};
	}

	EventDispatchResult ViewportClient::on_pointer_event(PointerEvent& event)
	{
		return {};
	}

	EventDispatchResult ViewportClient::on_gamepad_event(GamepadEvent& event)
	{
		return {};
	}

	EventDispatchResult ViewportClient::on_device_change_event(DeviceChangeEvent& event)
	{
		return {};
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
}// namespace Trinex
