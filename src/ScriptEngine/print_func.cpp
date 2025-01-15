#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>


namespace Engine
{
	static String to_string(asIScriptGeneric* gen, asUINT arg)
	{
		const void* address = gen->GetArgAddress(arg);
		int_t type_id       = gen->GetArgTypeId(arg);
		return ScriptEngine::to_string(reinterpret_cast<const byte*>(address), type_id, false);
	}

	static String to_string(StringView fmt, asIScriptGeneric* gen, int_t skip_args)
	{
		int_t argc = gen->GetArgCount() - skip_args;

		if (argc <= 0)
		{
			return String(fmt);
		}

		String result;
		result.reserve(fmt.size());

		for (size_t itr = 0, next = 0; itr < fmt.size(); itr = next)
		{
			next = fmt.find_first_of('%', itr);
			result += fmt.substr(itr, next - itr);

			if (next == std::string::npos)
				break;

			if (!isdigit(fmt[++next]))
				result += '%';
			else
			{
				auto arg = atoi(&fmt[next]) % argc;
				while (next < fmt.size() && isdigit(fmt[next])) ++next;
				arg += skip_args;

				const byte* address = reinterpret_cast<const byte*>(gen->GetArgAddress(arg));
				int_t type_id       = gen->GetArgTypeId(arg);
				result += ScriptEngine::to_string(address, type_id, false);
			}
		}

		return result;
	}

	static void string_fmt(asIScriptGeneric* gen)
	{
		const String* str = static_cast<const String*>(gen->GetObject());
		String result     = to_string(*str, gen, 0);
		new (gen->GetAddressOfReturnLocation()) String(std::move(result));
	}

	static void script_printf(asIScriptGeneric* gen)
	{
		const String* str = static_cast<const String*>(gen->GetArgAddress(0));
		String result     = to_string(*str, gen, 1);
		info_log("ScriptEngine", "%s", result.c_str());
	}

	static void script_print(asIScriptGeneric* gen)
	{
		asUINT count  = gen->GetArgCount();
		String result = count > 0 ? to_string(gen, 0) : "";

		for (asUINT arg = 1; arg < count; ++arg)
		{
			result += ", ";
			result += to_string(gen, arg);
		}

		info_log("ScriptEngine", "%s", result.c_str());
	}

	static void initialize()
	{
		auto& e  = ScriptEngine::instance();
		auto str = ScriptClassRegistrar::existing_class("string");

		str.method("string format(const ?& ...) const", string_fmt, ScriptCallConv::Generic);
		e.register_function("void print(const ?& ...)", script_print, ScriptCallConv::Generic);
		e.register_function("void printf(const string& fmt, const ?& ...)", script_printf, ScriptCallConv::Generic);
	}

	static Engine::PreInitializeController on_init(initialize, "Engine::PrintFunction", {"Engine::DefaultScriptAddons"});
}// namespace Engine
