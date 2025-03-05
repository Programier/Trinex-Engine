#pragma once
#include <Core/enums.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/string.hpp>

namespace Engine
{
	struct ENGINE_EXPORT WindowConfig {
		Set<WindowAttribute::Enum> attributes;
		Set<Orientation::Enum> orientations;

		String title;
		String client;
		Size2D size;
		Point2D position = {-1, -1};

		bool vsync;

		WindowConfig(bool init = true);
		WindowConfig& initialize();
		bool contains_attribute(WindowAttribute attribute) const;
	};
}// namespace Engine
