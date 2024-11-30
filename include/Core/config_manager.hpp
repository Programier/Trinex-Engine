#pragma once
#include <Core/engine_types.hpp>

namespace Engine::ConfigManager
{
	ENGINE_EXPORT bool load_config_from_text(const String& config);
	ENGINE_EXPORT bool load_config_from_file(const Path& file);
	ENGINE_EXPORT bool initialize();
}// namespace Engine::ConfigManager
