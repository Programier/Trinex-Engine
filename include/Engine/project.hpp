#pragma once
#include <Core/etl/string.hpp>
#include <Core/export.hpp>

namespace Engine
{
	class Path;
	struct ENGINE_EXPORT Project {
		static String name;
		static String version;

		// Project structure definition
		static String project_dir;
		static String resources_dir;
		static String configs_dir;
		static String assets_dir;
		static String scripts_dir;
		static String shaders_dir;
		static String localization_dir;
		static String libraries_dir;
		static String shader_cache_dir;

		static bool close_project();
		static bool open_project(const String& config, const Path& root);
		static bool open_project(const Path& project_file);

		static String to_string();
		static void initialize();
	};
}// namespace Engine
