#pragma once
#include <UI/types.hpp>

namespace Trinex::UI
{
	struct TabStyle {
		Unit rounding         = Unit(4.0f);
		Unit border_size      = Unit(0.0f);
		Unit min_width_base   = Unit(0.0f);
		Unit min_width_shrink = Unit(0.0f);
		Vec4 background_color = {0.11f, 0.15f, 0.17f, 1.00f};
		Vec4 overline_color   = {0.26f, 0.59f, 0.98f, 1.00f};

		TabStyle& push();
		TabStyle& pop();
	};
}// namespace Trinex::UI
