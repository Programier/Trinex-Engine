#include <Core/arguments.hpp>
#include <Core/entry_point.hpp>
#include <Core/etl/script_array.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <fstream>
#include <scripthelper.h>

namespace Engine
{
	class ScriptExec : public EntryPoint
	{
		trinex_declare_class(ScriptExec, EntryPoint);

	public:
		int_t execute() override
		{
			auto module_argument   = Arguments::find("module");
			auto function_argument = Arguments::find("function");

			if (module_argument == nullptr || module_argument->type != Arguments::Type::String)
			{
				error_log("ScriptExec", "Failed to get module name!");
				return -1;
			}

			if (function_argument == nullptr || function_argument->type != Arguments::Type::String)
			{
				error_log("ScriptExec", "Failed to get function name!");
				return -1;
			}


			ScriptModule module = ScriptEngine::module_by_name(module_argument->get<const String&>().c_str());

			if (!module.is_valid())
			{
				error_log("ScriptExec", "Failed to get script module!");
				return -1;
			}

			ScriptFunction function = module.function_by_name(function_argument->get<const String&>().c_str());

			if (!function.is_valid())
			{
				error_log("ScriptExec", "Failed to get script function!");
				return -1;
			}

			if (function.param_count() != 0)
			{
				error_log("ScriptExec", "Script function must have only 0 parameters");
				return -1;
			}

			int_t result = 0;
			ScriptContext::execute(function, &result);
			return result;
		}
	};

	class ScriptConfigDump : public EntryPoint
	{
		trinex_declare_class(ScriptConfigDump, EntryPoint);

	public:
		int_t execute() override
		{
			std::ofstream out_file("script_config.txt");
			if (out_file.is_open())
			{
				WriteConfigToStream(ScriptEngine::engine(), out_file);
			}
			return 0;
		}
	};

	trinex_implement_class_default_init(ScriptExec, 0);
	trinex_implement_class_default_init(ScriptConfigDump, 0);
}// namespace Engine
