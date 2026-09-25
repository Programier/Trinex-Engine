#pragma once
#include <Core/types/path.hpp>

namespace Trinex
{
	struct ENGINE_EXPORT Project {
		static String name;
		static String version;

		// Project structure definition
		static Path project_dir;
		static Path resources_dir;
		static Path configs_dir;
		static Path assets_dir;
		static Path scripts_dir;
		static Path shaders_dir;
		static Path localization_dir;
		static Path libraries_dir;
		static Path shader_cache_dir;

		static bool close_project();
		static bool open_project(const String& config, const Path& root);
		static bool open_project(const Path& project_file);

		static String to_string();
		static void initialize();
	};
}// namespace Trinex
