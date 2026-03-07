#pragma once

#include <Core/engine_types.hpp>

namespace Trinex
{
	struct ENGINE_EXPORT FileFlag final {
		union
		{
			usize data[2];
			char line[8 * (sizeof(data) / sizeof(data[0]))];
		};

		FileFlag(usize first = 0, usize second = 0);

		bool operator==(const FileFlag& other) const;
		bool operator!=(const FileFlag& other) const;


		static const FileFlag& package_flag();
		static const FileFlag& asset_flag();
	};
}// namespace Trinex
