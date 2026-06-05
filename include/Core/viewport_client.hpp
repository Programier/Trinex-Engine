#pragma once
#include <Core/object.hpp>

namespace Trinex
{
	struct EventDispatchResult;
	struct RoutedEvent;
	struct WindowEvent;
	struct KeyEvent;
	struct TextInputEvent;
	struct PointerEvent;
	struct GamepadEvent;
	struct DeviceChangeEvent;

	class ENGINE_EXPORT ViewportClient : public Object
	{
		trinex_class(ViewportClient, Object);

	public:
		virtual ViewportClient& attach(class RenderViewport* viewport);
		virtual ViewportClient& deattach(class RenderViewport* viewport);

		virtual EventDispatchResult on_event(RoutedEvent& event);
		virtual EventDispatchResult on_quit(RoutedEvent& event);
		virtual EventDispatchResult on_window_event(WindowEvent& event);
		virtual EventDispatchResult on_key_event(KeyEvent& event);
		virtual EventDispatchResult on_text_input_event(TextInputEvent& event);
		virtual EventDispatchResult on_pointer_event(PointerEvent& event);
		virtual EventDispatchResult on_gamepad_event(GamepadEvent& event);
		virtual EventDispatchResult on_device_change_event(DeviceChangeEvent& event);

		virtual ViewportClient& update(class RenderViewport* viewport, float dt);
		static ViewportClient* create(const StringView& name);
	};
}// namespace Trinex
