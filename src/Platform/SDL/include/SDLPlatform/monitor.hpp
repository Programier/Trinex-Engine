#pragma once
#include <Core/etl/flat_set.hpp>
#include <Platform/monitor.hpp>
#include <SDL3/SDL_video.h>


namespace Trinex::Platform
{
	class ENGINE_EXPORT SDLMonitorSystem final : public MonitorSystem
	{
	private:
		struct SDLMonitorData {
			SDL_DisplayID id = 0;
			Vector<MonitorMode> modes;
		};

		struct Compare {
			using is_transparent = void;

			inline bool operator()(const Monitor& a, const Monitor& b) const { return a.id < b.id; }
			inline bool operator()(u64 a, const Monitor& b) const { return a < b.id; }
			inline bool operator()(const Monitor& a, u64 b) const { return a.id < b; }

			inline bool operator()(const SDLMonitorData& a, const SDLMonitorData& b) const { return a.id < b.id; }
			inline bool operator()(u64 a, const SDLMonitorData& b) const { return a < b.id; }
			inline bool operator()(const SDLMonitorData& a, u64 b) const { return a.id < b; }
		};

		FlatSet<Monitor, Compare> m_monitors;
		FlatSet<SDLMonitorData, Compare> m_monitors_data;

	public:
		static SDLMonitorSystem* instance();

		SDLMonitorSystem();
		SDLMonitorSystem& on_change();

		const Monitor* monitors(usize& count) const override;
		const MonitorMode* modes(u32 monitor, usize& count) const override;

		u32 primary() const override;
		bool monitor(u32 monitor, Monitor* out) const override;
		bool mode(u32 monitor, MonitorMode* out) const override;
	};
}// namespace Trinex::Platform
