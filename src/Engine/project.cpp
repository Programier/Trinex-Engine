#include <Core/arguments.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Engine/project.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine
{
	String Project::name;
	String Project::version;

	// Project structure definition
	String Project::configs_dir;
	String Project::assets_dir;
	String Project::scripts_dir;
	String Project::shaders_dir;
	String Project::localization_dir;
	String Project::libraries_dir;
	String Project::shader_cache_dir;

	static void bind_to_script_engine()
	{
		static bool initialized = false;

		if (initialized)
			return;

		ScriptNamespaceScopedChanger changer("Engine::Project");

#define register_var(name) ScriptEngine::register_property("string " #name, &Project::name)

		register_var(name);
		register_var(version);
		register_var(configs_dir);
		register_var(assets_dir);
		register_var(scripts_dir);
		register_var(shaders_dir);
		register_var(localization_dir);
		register_var(libraries_dir);
		register_var(shader_cache_dir);

		initialized = true;
	}


	bool Project::close_project()
	{
		name             = "";
		version          = "";
		configs_dir      = "";
		assets_dir       = "";
		scripts_dir      = "";
		shaders_dir      = "";
		localization_dir = "";
		libraries_dir    = "";
		shader_cache_dir = "";
		return true;
	}

	static void create_folders()
	{
		auto rfs = rootfs();

		rfs->create_dir(Project::configs_dir);
		rfs->create_dir(Project::assets_dir);
		rfs->create_dir(Project::scripts_dir);
		rfs->create_dir(Project::shaders_dir);
		rfs->create_dir(Project::localization_dir);
		rfs->create_dir(Project::libraries_dir);
		rfs->create_dir(Project::shader_cache_dir);
	}

	static void rename_dirs_to_mount_points()
	{
		Project::configs_dir      = "[configs_dir]:";
		Project::assets_dir       = "[assets_dir]:";
		Project::scripts_dir      = "[scripts_dir]:";
		Project::shaders_dir      = "[shaders_dir]:";
		Project::localization_dir = "[localization_dir]:";
		Project::libraries_dir    = "[libraries_dir]:";
		Project::shader_cache_dir = "[shader_cache_dir]:";
	}

	static void apply_project_config()
	{
		auto rfs = rootfs();
		create_folders();

		rfs->mount("[configs_dir]:", Project::configs_dir);
		rfs->mount("[assets_dir]:", Project::assets_dir);
		rfs->mount("[scripts_dir]:", Project::scripts_dir);
		rfs->mount("[shaders_dir]:", Project::shaders_dir);
		rfs->mount("[localization_dir]:", Project::localization_dir);
		rfs->mount("[libraries_dir]:", Project::libraries_dir);
		rfs->mount("[shader_cache_dir]:", Project::shader_cache_dir);

		rename_dirs_to_mount_points();
	}

	bool Project::open_project(const String& config, const Path& root)
	{
		if (close_project() == false)
			return false;
		bool status = ScriptEngine::exec_string(config);


		if (status)
		{
			configs_dir      = (root / configs_dir).str();
			assets_dir       = (root / assets_dir).str();
			scripts_dir      = (root / scripts_dir).str();
			shaders_dir      = (root / shaders_dir).str();
			localization_dir = (root / localization_dir).str();
			libraries_dir    = (root / libraries_dir).str();
			shader_cache_dir = (root / shader_cache_dir).str();
			apply_project_config();
		}

		return status;
	}

	bool Project::open_project(const Path& project_file)
	{
		FileReader reader(project_file);
		if (!reader.is_open())
			return false;
		return open_project(reader.read_string(), project_file.base_path());
	}

	static constexpr inline const char* literal = R"(// Trinex Engine Project file
Engine::Project::name = "{}";
Engine::Project::version = "{}";
Engine::Project::configs_dir = "{}";
Engine::Project::assets_dir = "{}";
Engine::Project::scripts_dir = "{}";
Engine::Project::shaders_dir = "{}";
Engine::Project::localization_dir = "{}";
Engine::Project::libraries_dir = "{}";
Engine::Project::shader_cache_dir = "{}";
)";

	String Project::to_string()
	{
		return Strings::format(literal, name, version, configs_dir, assets_dir, scripts_dir, shader_cache_dir, localization_dir,
		                       libraries_dir, shader_cache_dir);
	}

	static bool check_initialize(bool with_msg)
	{
#define check_var(var)                                                                                                           \
	if (Project::var.empty())                                                                                                    \
	{                                                                                                                            \
		if (with_msg)                                                                                                            \
		{                                                                                                                        \
			error_log("Project", "Project value '%s' can't be empty!", #var);                                                    \
		}                                                                                                                        \
		return false;                                                                                                            \
	}
		check_var(configs_dir);
		check_var(assets_dir);
		check_var(scripts_dir);
		check_var(shaders_dir);
		check_var(localization_dir);
		check_var(libraries_dir);
		check_var(shader_cache_dir);
		return true;
	}

	static void setup_default_project()
	{
		Engine::Project::name             = "Trinex Engine Project";
		Engine::Project::version          = "1.0.0";
		Engine::Project::configs_dir      = "resources/configs";
		Engine::Project::assets_dir       = "resources/assets";
		Engine::Project::scripts_dir      = "resources/scripts";
		Engine::Project::shaders_dir      = "resources/shaders";
		Engine::Project::localization_dir = "resources/localization";
		Engine::Project::libraries_dir    = "libs";
		Engine::Project::shader_cache_dir = "ShaderCache";

		apply_project_config();
	}

	void Project::initialize()
	{
		bind_to_script_engine();

		// Try to load project using files from argument
		auto arg = Arguments::find("project");

		if (arg && arg->type == Arguments::Type::String)
		{
			if (open_project(arg->get<String>()) && check_initialize(false))
				return;
		}

		if (!open_project("TrinexProject.trinex") || !check_initialize(true))
		{
			error_log("Project", "Failed to initialize project. Using default project settins!");
			setup_default_project();
		}
	}
}// namespace Engine
