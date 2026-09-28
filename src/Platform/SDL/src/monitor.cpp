#include <SDL3/SDL.h>
#include <SDLPlatform/monitor.hpp>
#include <cmath>

namespace Trinex::Platform
{
	static u32 refresh_rate(const SDL_DisplayMode& mode)
	{
		// Prefer the precise rational representation.
		if (mode.refresh_rate_numerator > 0 && mode.refresh_rate_denominator > 0)
		{
			const u64 numerator = static_cast<u64>(mode.refresh_rate_numerator) * 1000ull;

			const u64 denominator = static_cast<u64>(mode.refresh_rate_denominator);

			// Hz -> mHz, rounded to nearest integer.
			return static_cast<u32>((numerator + denominator / 2) / denominator);
		}

		// Fallback to the floating-point value.
		if (mode.refresh_rate > 0.f)
			return static_cast<u32>(std::lround(mode.refresh_rate * 1000.f));

		return 0;
	}

	static void convert_mode(SDL_DisplayID display, const SDL_DisplayMode& src, MonitorMode* out)
	{
		out->monitor       = display;
		out->resolution    = {static_cast<u32>(src.w), static_cast<u32>(src.h)};
		out->pixel_density = src.pixel_density > 0.f ? src.pixel_density : 1.f;
		out->refresh_rate  = refresh_rate(src);
	}

	SDLMonitorSystem* SDLMonitorSystem::instance()
	{
		static SDLMonitorSystem system;
		return &system;
	}

	SDLMonitorSystem::SDLMonitorSystem()
	{
		on_change();
	}

	SDLMonitorSystem& SDLMonitorSystem::on_change()
	{
		m_monitors.clear();
		m_monitors_data.clear();

		int displays_count      = 0;
		SDL_DisplayID* displays = SDL_GetDisplays(&displays_count);

		if (!displays)
			return *this;

		m_monitors.reserve(displays_count);
		m_monitors_data.reserve(displays_count);

		for (int display = 0; display < displays_count; ++display)
		{
			{
				Monitor info;

				if (monitor(displays[display], &info))
				{
					m_monitors.insert(static_cast<Monitor&&>(info));
				}
			}
			{
				SDLMonitorData monitor;
				monitor.id = displays[display];

				int modes_count         = 0;
				SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(monitor.id, &modes_count);

				if (modes == nullptr)
					continue;

				monitor.modes.reserve(modes_count);

				for (int mode_index = 0; mode_index < modes_count; ++mode_index)
				{
					auto& mode = monitor.modes.emplace_back();
					convert_mode(monitor.id, *modes[mode_index], &mode);
				}

				SDL_free(modes);
				m_monitors_data.insert(static_cast<SDLMonitorData&&>(monitor));
			}
		}


		SDL_free(displays);
		return *this;
	}

	const Monitor* SDLMonitorSystem::monitors(usize& count) const
	{
		count = m_monitors.size();
		return m_monitors.container().data();
	}

	const MonitorMode* SDLMonitorSystem::modes(u32 monitor, usize& count) const
	{
		auto it = m_monitors_data.find(monitor);

		if (it == m_monitors_data.end())
		{
			count = 0;
			return nullptr;
		}

		count = it->modes.size();
		return it->modes.data();
	}

	u32 SDLMonitorSystem::primary() const
	{
		return SDL_GetPrimaryDisplay();
	}

	bool SDLMonitorSystem::monitor(u32 monitor, Monitor* out) const
	{
		if (!out || monitor == 0)
			return false;

		SDL_Rect bounds = {};
		if (!SDL_GetDisplayBounds(monitor, &bounds))
			return false;

		SDL_Rect work_bounds = {};
		if (!SDL_GetDisplayUsableBounds(monitor, &work_bounds))
			return false;

		if (!mode(monitor, &out->mode))
			return false;

		const f32 content_scale = SDL_GetDisplayContentScale(monitor);

		out->id            = monitor;
		out->name          = SDL_GetDisplayName(monitor);
		out->position      = {bounds.x, bounds.y};
		out->size          = {static_cast<u32>(bounds.w), static_cast<u32>(bounds.h)};
		out->work_position = {work_bounds.x, work_bounds.y};
		out->work_size     = {static_cast<u32>(work_bounds.w), static_cast<u32>(work_bounds.h)};
		out->content_scale = content_scale > 0.f ? content_scale : 1.f;
		out->primary       = monitor == SDL_GetPrimaryDisplay();
		return true;
	}

	bool SDLMonitorSystem::mode(u32 monitor, MonitorMode* out) const
	{
		const SDL_DisplayMode* src = SDL_GetCurrentDisplayMode(monitor);

		if (!src)
			return false;

		convert_mode(monitor, *src, out);
		return true;
	}

}// namespace Trinex::Platform
