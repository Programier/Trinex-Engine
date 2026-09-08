#pragma once
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Platform/enums.hpp>
#include <Platform/types.hpp>

namespace Trinex::Platform
{
	struct DisplayMode {
		Vector2u size      = {0, 0};
		u32 refresh_rate   = 0;
		u32 bits_per_pixel = 0;
	};

	struct MonitorInfo {
		Identifier id = 0;
		String name;
		Vector2i position      = {0, 0};
		Vector2u pos           = {0, 0};
		Vector2u size          = {0, 0};
		Vector2u physical_size = {0, 0};
		float dpi              = 0.f;
		Vector2f dpi_scale     = {1.f, 1.f};
		DisplayMode current_mode;
		bool is_primary = false;
	};

	class ENGINE_EXPORT DisplaySystem
	{
	public:
		static DisplaySystem* instance();

		virtual ~DisplaySystem() = default;

		virtual usize monitors_count() const                                            = 0;
		virtual bool monitor_info(usize monitor_index, MonitorInfo* out) const          = 0;
		virtual bool display_modes(usize monitor_index, Vector<DisplayMode>* out) const = 0;
	};
}// namespace Trinex::Platform
