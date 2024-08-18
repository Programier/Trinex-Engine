#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

namespace Engine
{
	enum class EventType : byte
	{
		Undefined,

		// Application events
		Quit,
		AppTerminating,
		AppLowMemory,
		AppPause,
		AppResume,

		TextInput,

		// Display events
		DisplayAdded,
		DisplayRemoved,
		DisplayOrientationChanged,

		// Window events
		WindowShown,//
		WindowHidden,
		WindowMoved,
		WindowResized,
		WindowMinimized,
		WindowMaximized,

		WindowRestored,
		WindowFocusGained,
		WindowFocusLost,
		WindowClose,

		// Keyboard events
		KeyDown,
		KeyUp,

		// Mouse events
		MouseMotion,
		MouseButtonUp,
		MouseButtonDown,
		MouseWheel,

		// Game controller events
		ControllerAxisMotion,
		ControllerButtonUp,
		ControllerButtonDown,
		ControllerDeviceAdded,
		ControllerDeviceRemoved,
		ControllerDeviceRemapped,
		ControllerTouchPadDown,
		ControllerTouchPadMotion,
		ControllerTouchPadUp,
		ControllerSensorUpdate,

		// Touch events
		FingerDown,
		FingerUp,
		FingerMotion,

		// Drag and drop events
		DropFile,
		DropText,
		DropBegin,
		DropComplete,
	};


	class ENGINE_EXPORT Event
	{
	private:
		Any m_any;
		Identifier m_window_id;
		EventType m_type;


	public:
		Event();
		copy_constructors_hpp(Event);
		Event(Identifier window_id, EventType type);
		Event(Identifier window_id, EventType type, const Any& any);

		template<typename T>
		Event(Identifier window_id, EventType type, T&& value) : Event(window_id, type)
		{
			m_any = std::forward<T>(value);
		}

		EventType type() const;
		Identifier window_id() const;
		const Any& any() const;

		template<typename Type>
		Type get()
		{
			return m_any.cast<Type>();
		}

		template<typename Type>
		const Type get() const
		{
			return m_any.cast<const Type>();
		}
	};
}// namespace Engine
