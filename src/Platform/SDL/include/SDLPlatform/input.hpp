#pragma once
#include <Core/etl/string.hpp>
#include <Core/etl/array.hpp>
#include <Input/device.hpp>
#include <Platform/input.hpp>

namespace Trinex::Platform
{
	class SDLInputDevice : public InputDevice
	{
	private:
		InputDeviceType m_type;
		u64 m_id;
		String m_name;
		Array<InputEvent> m_state;

	public:
		SDLInputDevice(InputDeviceType type, u64 id, String name);

		const char* name() const override;
		u32 types() const override;
		InputDeviceType type(u32 index = 0) const override;
		const InputEvent* state(u32& len) const override;

		u64 id() const;
		void emit(InputEvent event);
	};

	class trinex_api SDLInputSystem : public InputSystem
	{
	private:
		Array<Ref<SDLInputDevice>> m_devices;

	public:
		static SDLInputSystem* instance();

		void update();
		usize devices() const override;
		WeakRef<InputDevice> device(usize idx) const override;
		bool text_input_enabled() const override;
		InputSystem& text_input_enabled(bool enabled) override;
	};
}// namespace Trinex::Platform
