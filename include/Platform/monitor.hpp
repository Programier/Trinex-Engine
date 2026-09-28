#pragma once
#include <Core/math/vector.hpp>
#include <Platform/enums.hpp>

namespace Trinex::Platform
{
	struct MonitorMode {
		u32 monitor         = 0;     // Monitor identifier
		Vector2u resolution = {0, 0};// Video mode size in display coordinates.
		f32 pixel_density   = 1.f;   // Scale from display coordinates to physical pixels.
		u32 refresh_rate    = 0;     // Refresh rate in millihertz (mHz).
	};

	struct Monitor {
		const char* name = nullptr;// Human-readable monitor name.
		u32 id           = 0;      // Stable monitor identifier.

		Vector2i position = {0, 0};// Monitor position in virtual desktop coordinates.
		Vector2u size     = {0, 0};// Monitor size in desktop coordinate units.

		Vector2i work_position = {0, 0};// Position of the usable desktop area, excluding panels/taskbars.
		Vector2u work_size     = {0, 0};// Size of the usable desktop area, excluding panels/taskbars.

		f32 content_scale = 1.f;// User/OS preferred content scaling factor for this monitor.

		MonitorMode mode = {};   // Currently active video mode.
		bool primary     = false;// Whether this is the primary monitor.
	};

	class ENGINE_EXPORT MonitorSystem
	{
	public:
		static MonitorSystem* instance();

		virtual ~MonitorSystem() = default;

		virtual const Monitor* monitors(usize& count) const               = 0;
		virtual const MonitorMode* modes(u32 monitor, usize& count) const = 0;

		virtual u32 primary() const                            = 0;
		virtual bool monitor(u32 monitor, Monitor* out) const  = 0;
		virtual bool mode(u32 monitor, MonitorMode* out) const = 0;

		inline bool monitor(Monitor* out) const { return monitor(primary(), out); }
		inline const MonitorMode* modes(usize& count) const { return modes(primary(), count); }
		inline bool mode(MonitorMode* out) const { return mode(primary(), out); }


		template<typename F>
		inline const MonitorSystem& for_each_monitor(F&& f) const
		{
			usize count;
			const Monitor* monitor = monitors(count);

			for (usize i = 0; i < count; ++i)
			{
				f(monitor[i]);
			}

			return *this;
		}

		template<typename F>
		inline const MonitorSystem& for_each_mode(u32 monitor, F&& f) const
		{
			usize count             = 0;
			const MonitorMode* data = modes(monitor, count);

			for (usize i = 0; i < count; ++i) f(data[i]);
			return *this;
		}

		template<typename F>
		inline const MonitorSystem& for_each_mode(F&& f) const
		{
			return for_each_mode(primary(), f);
		}
	};
}// namespace Trinex::Platform
