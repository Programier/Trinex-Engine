#pragma once
#include <Core/etl/script_array.hpp>

namespace Engine
{
	struct ENGINE_EXPORT Project {
		static String name;
		static String version;

		// Project structure definition
		static String configs_dir;
		static String assets_dir;
		static String scripts_dir;
		static String shaders_dir;
		static String localization_dir;
		static String libraries_dir;
		static String shader_cache_dir;

		// Setup this variable using PreInitializeController, if you need to use custom project initialization
		// By default engine will be read file "TrinexProject.trinex" in executable directory
		static String (*custom_project_config)();

		static bool close_project();
		static bool open_project(const String& config, const Path& root);
		static bool open_project(const Path& project_file);

		static String to_string();
		static void initialize();
	};
}// namespace Engine
