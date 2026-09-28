#pragma once
#include <string>
#include <string_view>

namespace Trinex
{
	using String     = std::string;
	using StringView = std::string_view;

	ENGINE_EXPORT void serialize(class Archive& ar, String& string);
}// namespace Trinex
