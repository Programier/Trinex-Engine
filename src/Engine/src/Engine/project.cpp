#include <Core/arguments.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Engine/project.hpp>
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

    String (*Project::custom_project_config)() = nullptr;

    static void bind_to_script_engine()
    {
        static bool initialized = false;

        if (initialized)
            return;

        ScriptEngine::NamespaceSaverScoped ns_saver;

        ScriptEngine::default_namespace("Engine::Project");
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

    bool Project::open_project(const String& config, const Path& root)
    {
        if (close_project() == false)
            return false;
        return ScriptEngine::exec_string(config);
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

    void Project::initialize()
    {
        bind_to_script_engine();

        // if (custom_project_config)
        // {
        //     String source = custom_project_config();
        //     if (from_string(source) && check_initialize(false))
        //         return;
        // }

        // Try to load project using files from argument
        auto arg = Arguments::find("project");

        if (arg && arg->type == Arguments::Type::String)
        {
            if (open_project(arg->get<String>()) && check_initialize(false))
                return;
        }

        if (!open_project("TrinexProject.trinex") || !check_initialize(true))
        {
            throw EngineException("Failed to initialize project!");
        }
    }
}// namespace Engine
