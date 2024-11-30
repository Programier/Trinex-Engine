#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Engine/project.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>
#include <scriptbuilder.h>

namespace Engine::ConfigManager
{
#define check_result()                                                                                                           \
	if (r < 0)                                                                                                                   \
	return false

	using CompiledConfigs = TreeSet<String>;

	static String read_config(const Path& config_file)
	{
		FileReader reader(Path(Project::configs_dir) / config_file);
		if (!reader.is_open())
		{
			warn_log("ConfigManager", "Failed to load config '%s'", config_file.c_str());
			return "";
		}

		return reader.read_string();
	}

	static int include_config(const char* include, const char* from, CScriptBuilder* builder, void* user_param)
	{
		CompiledConfigs& compiled = *reinterpret_cast<CompiledConfigs*>(user_param);

		String config_file = include;

		if (compiled.contains(config_file))
		{
			return 0;
		}

		String config = read_config(config_file);

		if (config.empty())
		{
			warn_log("Config", "Config code is empty!");
			return -1;
		}

		compiled.insert(std::move(config_file));
		return builder->AddSectionFromMemory(include, config.c_str(), config.length());
	}

	ENGINE_EXPORT bool load_config_from_text(const String& config, CompiledConfigs& compiled)
	{
		if (config.empty())
		{
			warn_log("Config", "Config code is empty!");
			return true;// It theory, this should not be a critical error
		}

		CScriptBuilder builder;
		builder.SetIncludeCallback(include_config, &compiled);

		auto engine = ScriptEngine::engine();
		int r       = builder.StartNewModule(engine, "__TRINEX_ENGINE_CONFIGS__");

		check_result();
		const char* section = compiled.empty() ? "__TRINEX_ENGINE_ROOT_CONFIG__" : compiled.begin()->c_str();
		builder.AddSectionFromMemory(section, config.c_str(), config.length());
		check_result();
		builder.BuildModule();
		check_result();

		auto module  = builder.GetModule();
		asUINT funcs = module->GetFunctionCount();

		const StringView find_entry = "config_main";
		for (asUINT i = 0; i < funcs; ++i)
		{
			auto func = module->GetFunctionByIndex(i);

			if (func->GetParamCount() == 0)
			{
				auto metadata = builder.GetMetadataForFunc(func);

				if (std::find(metadata.begin(), metadata.end(), find_entry) != metadata.end())
				{
					ScriptContext::execute(func);
				}
			}
		}

		module->Discard();
		return true;
	}

	ENGINE_EXPORT bool load_config_from_text(const String& config)
	{
		CompiledConfigs compiled;
		return load_config_from_text(config, compiled);
	}

	ENGINE_EXPORT bool load_config_from_file(const Path& file)
	{
		String config            = read_config(file);
		CompiledConfigs compiled = {file.c_str()};
		return load_config_from_text(config, compiled);
	}

	ENGINE_EXPORT bool initialize()
	{
		auto& e        = ScriptEngine::instance();
		auto var_count = e.global_property_count();

		Set<StringView> pending;

		for (uint_t i = 0; i < var_count; ++i)
		{
			StringView group;
			e.global_property(i, nullptr, nullptr, &group);

			if (group.ends_with(".config"))
			{
				pending.emplace(group);
			}
		}

		CompiledConfigs compiled;

		while (!pending.empty())
		{
			String group = String(*pending.begin());
			pending.erase(pending.begin());

			if (!compiled.contains(group))
			{
				const String& group_ref = *compiled.insert(std::move(group)).first;
				if (!load_config_from_text(read_config(group_ref), compiled))
					return false;
			}
		}

		ConfigsInitializeController().execute();
		return true;
	}
}// namespace Engine::ConfigManager
