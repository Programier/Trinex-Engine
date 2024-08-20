#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/stacktrace.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>
#include <scripthelper.h>

#if ARCH_ARM
#include "jit_compiler/arm64/compiler.hpp"
using PlatformJitCompiler = JIT::ARM64_Compiler;

#elif ARCH_X86_64
#include "jit_compiler/x86-64/compiler.hpp"
using PlatformJitCompiler = JIT::X86_64_Compiler;
#endif

static constexpr bool enable_jit = false;

namespace Engine
{
	static void angel_script_callback(const asSMessageInfo* msg, void* param)
	{
		if (msg->type == asMSGTYPE_WARNING)
		{
			warn_log("ScriptEngine", "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
		}
		else if (msg->type == asMSGTYPE_INFORMATION)
		{
			info_log("ScriptEngine", "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
		}
		else
		{
			if (ScriptEngine::exception_on_error)
				throw EngineException(Strings::format("{} ({}, {}): {}", msg->section, msg->row, msg->col, msg->message));
			else
				error_log("ScriptEngine", "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
		}
	}


	Vector<class Script*> ScriptEngine::m_scripts;
	asIScriptEngine* ScriptEngine::m_engine		 = nullptr;
	asIJITCompiler* ScriptEngine::m_jit_compiler = nullptr;
	ScriptFolder* ScriptEngine::m_script_folder	 = nullptr;
	TreeMap<int_t, ScriptEngine::VariableToStringFunction> ScriptEngine::m_custom_variable_parsers;

	bool ScriptEngine::exception_on_error = true;
	CallBacks<void()> ScriptEngine::on_terminate;

	ScriptNamespaceScopedChanger::ScriptNamespaceScopedChanger() : m_prev_namespace(ScriptEngine::default_namespace())
	{}

	ScriptNamespaceScopedChanger::ScriptNamespaceScopedChanger(const char* new_namespace) : ScriptNamespaceScopedChanger()
	{
		ScriptEngine::default_namespace(new_namespace);
	}

	ScriptNamespaceScopedChanger::ScriptNamespaceScopedChanger(const String& new_namespace)
		: ScriptNamespaceScopedChanger(new_namespace.c_str())
	{}

	const String& ScriptNamespaceScopedChanger::saved_namespace() const
	{
		return m_prev_namespace;
	}

	ScriptNamespaceScopedChanger::~ScriptNamespaceScopedChanger()
	{
		ScriptEngine::default_namespace(m_prev_namespace);
	}

	ScriptEngine& ScriptEngine::initialize()
	{
		if (m_engine != nullptr)
			return instance();

		m_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		info_log("ScriptEngine", "Created script engine [%p]", m_engine);

		m_engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
		m_engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, 1);
		m_engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
		m_engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);
		m_engine->SetMessageCallback(asFUNCTION(angel_script_callback), 0, asCALL_CDECL);

#if ARCH_X86_64 || ARCH_ARM
		if constexpr (enable_jit)
		{
			info_log("ScriptEngine", "Enable JIT compiler!");
			auto compiler  = new PlatformJitCompiler();
			m_jit_compiler = compiler;
			m_engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, true);
			m_engine->SetJITCompiler(m_jit_compiler);
		}
#endif
		PostDestroyController controller(ScriptEngine::terminate, "Engine::ScriptEngine");
		ScriptContext::initialize();

		ScriptAddonsInitializeController().execute();
		m_script_folder = new ScriptFolder("scripts:");
		return instance();
	}

	void ScriptEngine::terminate()
	{
		if (m_engine)
		{
			on_terminate();
			ScriptContext::terminate();
			delete m_script_folder;
			m_script_folder = nullptr;
			m_engine->Release();
			m_engine = nullptr;
		}

		if (m_jit_compiler)
		{
			delete m_jit_compiler;
			m_jit_compiler = nullptr;
		}
	}

	ScriptEngine& ScriptEngine::instance()
	{
		static ScriptEngine engine;
		return engine;
	}

	asIScriptEngine* ScriptEngine::engine()
	{
		return m_engine;
	}

	asIScriptContext* ScriptEngine::new_context()
	{
		return m_engine->RequestContext();
	}

	static asDWORD create_call_conv(ScriptCallConv conv)
	{
		switch (conv)
		{
			case ScriptCallConv::CDecl:
				return asCALL_CDECL;
			case ScriptCallConv::StdCall:
				return asCALL_STDCALL;
			case ScriptCallConv::ThisCallAsGlobal:
				return asCALL_THISCALL_ASGLOBAL;
			case ScriptCallConv::ThisCall:
				return asCALL_THISCALL;
			case ScriptCallConv::CDeclObjLast:
				return asCALL_CDECL_OBJLAST;
			case ScriptCallConv::CDeclObjFirst:
				return asCALL_CDECL_OBJFIRST;
			case ScriptCallConv::Generic:
				return asCALL_GENERIC;
			case ScriptCallConv::ThisCall_ObjLast:
				return asCALL_THISCALL_OBJLAST;
			case ScriptCallConv::ThisCall_ObjFirst:
				return asCALL_THISCALL_OBJFIRST;
			default:
				throw EngineException("Undefined call convension!");
		}
	}

	ScriptFunction ScriptEngine::register_function(const char* declaration, ScriptFuncPtr* func, ScriptCallConv conv)
	{
		int_t id = m_engine->RegisterGlobalFunction(declaration, *reinterpret_cast<asSFuncPtr*>(func), create_call_conv(conv));
		if (id < 0)
			return {};
		return function_by_id(id);
	}

	ScriptFunction ScriptEngine::register_function(const String& declaration, ScriptFuncPtr* func, ScriptCallConv conv)
	{
		return register_function(declaration.c_str(), func, conv);
	}

	const ScriptEngine& ScriptEngine::release_context(asIScriptContext* context)
	{
		m_engine->ReturnContext(context);
		return instance();
	}

	ScriptEngine& ScriptEngine::default_namespace(const String& name)
	{
		return default_namespace(name.c_str());
	}

	ScriptEngine& ScriptEngine::default_namespace(const char* ns)
	{
		m_engine->SetDefaultNamespace(ns);
		return instance();
	}

	StringView ScriptEngine::default_namespace()
	{
		return m_engine->GetDefaultNamespace();
	}

	int_t ScriptEngine::register_property(const char* declaration, void* data)
	{
		return m_engine->RegisterGlobalProperty(declaration, data);
	}

	int_t ScriptEngine::register_property(const String& declaration, void* data)
	{
		return register_property(declaration.c_str(), data);
	}

	ScriptModule ScriptEngine::create_module(const String& name, EnumerateType flags)
	{
		return create_module(name.c_str(), flags);
	}

	ScriptModule ScriptEngine::create_module(const char* name, EnumerateType flags)
	{
		using MFlags = ScriptModule::ModuleFlags;

		MFlags module_flags = static_cast<MFlags>(flags);
		switch (module_flags)
		{
			case MFlags::CreateIfNotExists:
				return m_engine->GetModule(name, asGM_CREATE_IF_NOT_EXISTS);
			case MFlags::OnlyIfExists:
				return m_engine->GetModule(name, asGM_ONLY_IF_EXISTS);
			case MFlags::AlwaysCreate:
				return m_engine->GetModule(name, asGM_ALWAYS_CREATE);
			default:
				return ScriptModule();
		}
	}

	uint_t ScriptEngine::module_count()
	{
		return m_engine->GetModuleCount();
	}

	class ScriptFolder* ScriptEngine::scripts_folder()
	{
		return m_script_folder;
	}

	ScriptEngine& ScriptEngine::bind_imports()
	{
		for (Counter i = 0, j = m_engine->GetModuleCount(); i < j; i++)
		{
			m_engine->GetModuleByIndex(i)->BindAllImportedFunctions();
		}
		return instance();
	}

	ScriptEngine& ScriptEngine::funcdef(const char* declaration)
	{
		m_engine->RegisterFuncdef(declaration);
		return instance();
	}

	ScriptEngine& ScriptEngine::funcdef(const String& declaration)
	{
		return funcdef(declaration.c_str());
	}

	ScriptEngine& ScriptEngine::register_typedef(const char* type, const char* declaration)
	{
		m_engine->RegisterTypedef(type, declaration);
		return instance();
	}

	ScriptEngine& ScriptEngine::register_typedef(const String& type, const String& declaration)
	{
		return register_typedef(type.c_str(), declaration.c_str());
	}

	ScriptEngine& ScriptEngine::destroy_script_object(ScriptObjectAddress object, const ScriptTypeInfo& info)
	{
		m_engine->ReleaseScriptObject(object, info.info());
		return instance();
	}

	uint_t ScriptEngine::global_function_count()
	{
		return m_engine->GetGlobalFunctionCount();
	}

	ScriptFunction ScriptEngine::global_function_by_index(uint_t index)
	{
		return ScriptFunction(m_engine->GetGlobalFunctionByIndex(index));
	}

	ScriptFunction ScriptEngine::global_function_by_decl(const char* declaration)
	{
		return ScriptFunction(m_engine->GetGlobalFunctionByDecl(declaration));
	}

	ScriptFunction ScriptEngine::global_function_by_decl(const String& declaration)
	{
		return global_function_by_decl(declaration.c_str());
	}

	uint_t ScriptEngine::global_property_count()
	{
		return m_engine->GetGlobalPropertyCount();
	}

	int_t ScriptEngine::global_property_index_by_name(const char* name)
	{
		return m_engine->GetGlobalPropertyIndexByName(name);
	}

	int_t ScriptEngine::global_property_index_by_name(const String& name)
	{
		return global_property_index_by_name(name.c_str());
	}

	int_t ScriptEngine::global_property_index_by_decl(const char* declaration)
	{
		return m_engine->GetGlobalPropertyIndexByDecl(declaration);
	}

	int_t ScriptEngine::global_property_index_by_decl(const String& declaration)
	{
		return global_property_index_by_decl(declaration.c_str());
	}

	bool ScriptEngine::global_property(uint_t index, StringView* name, StringView* name_space, int_t* type_id, bool* is_const,
									   byte** pointer)
	{
		const char* c_name		 = nullptr;
		const char* c_name_space = nullptr;

		bool result = m_engine->GetGlobalPropertyByIndex(index, name ? &c_name : nullptr, name_space ? &c_name_space : nullptr,
														 type_id, is_const, nullptr, reinterpret_cast<void**>(pointer));

		if (result)
		{
			if (name)
			{
				*name = c_name;
			}

			if (name_space)
			{
				*name_space = c_name_space;
			}
		}

		return result;
	}


	ScriptEngine& ScriptEngine::garbage_collect(BitMask flags, size_t iterations)
	{
		m_engine->GarbageCollect(flags, iterations);
		return instance();
	}

	uint_t ScriptEngine::object_type_count()
	{
		return m_engine->GetObjectTypeCount();
	}

	ScriptTypeInfo ScriptEngine::object_type_by_index(uint_t index)
	{
		return ScriptTypeInfo(m_engine->GetObjectTypeByIndex(index));
	}

	bool ScriptEngine::exec_string(const String& line)
	{
		return exec_string(line.c_str());
	}

	bool ScriptEngine::exec_string(const char* line)
	{
		return ExecuteString(m_engine, line) >= 0;
	}

	// Enums
	uint_t ScriptEngine::enum_count()
	{
		return m_engine->GetEnumCount();
	}

	ScriptTypeInfo ScriptEngine::enum_by_index(uint_t index)
	{
		return ScriptTypeInfo(m_engine->GetEnumByIndex(index));
	}

	// Funcdefs
	uint_t ScriptEngine::funcdef_count()
	{
		return m_engine->GetFuncdefCount();
	}

	ScriptTypeInfo ScriptEngine::funcdef_by_index(uint_t index)
	{
		return ScriptTypeInfo(m_engine->GetFuncdefByIndex(index));
	}

	// Typedefs
	uint_t ScriptEngine::typedef_count()
	{
		return m_engine->GetTypedefCount();
	}

	ScriptTypeInfo ScriptEngine::typedef_by_index(uint_t index)
	{
		return ScriptTypeInfo(m_engine->GetTypedefByIndex(index));
	}

	// Script modules
	ScriptEngine& ScriptEngine::discard_module(const char* module_name)
	{
		m_engine->DiscardModule(module_name);
		return instance();
	}

	ScriptEngine& ScriptEngine::discard_module(const String& module_name)
	{
		return discard_module(module_name.c_str());
	}

	ScriptModule ScriptEngine::module_by_index(uint_t index)
	{
		return ScriptModule(m_engine->GetModuleByIndex(index));
	}

	// Script functions
	int_t ScriptEngine::last_function_id()
	{
		return m_engine->GetLastFunctionId();
	}

	ScriptFunction ScriptEngine::function_by_id(int_t func_id)
	{
		return ScriptFunction(m_engine->GetFunctionById(func_id));
	}

	// Type identification

	bool ScriptEngine::is_primitive_type(int_t type_id)
	{
		return sizeof_primitive_type(type_id) != 0;
	}

	bool ScriptEngine::is_bool(int_t type_id)
	{
		return type_id == asTYPEID_BOOL;
	}

	bool ScriptEngine::is_int8(int_t type_id)
	{
		return type_id == asTYPEID_INT8;
	}

	bool ScriptEngine::is_int16(int_t type_id)
	{
		return type_id == asTYPEID_INT16;
	}

	bool ScriptEngine::is_int32(int_t type_id)
	{
		return type_id == asTYPEID_INT32;
	}

	bool ScriptEngine::is_int64(int_t type_id)
	{
		return type_id == asTYPEID_INT64;
	}

	bool ScriptEngine::is_uint8(int_t type_id)
	{
		return type_id == asTYPEID_UINT8;
	}

	bool ScriptEngine::is_uint16(int_t type_id)
	{
		return type_id == asTYPEID_UINT16;
	}

	bool ScriptEngine::is_uint32(int_t type_id)
	{
		return type_id == asTYPEID_UINT32;
	}

	bool ScriptEngine::is_uint64(int_t type_id)
	{
		return type_id == asTYPEID_UINT64;
	}

	bool ScriptEngine::is_float(int_t type_id)
	{
		return type_id == asTYPEID_FLOAT;
	}

	bool ScriptEngine::is_double(int_t type_id)
	{
		return type_id == asTYPEID_DOUBLE;
	}


	bool ScriptEngine::is_object_type(int_t type_id, bool handle_is_object)
	{
		return (type_id & asTYPEID_MASK_OBJECT) && (handle_is_object ? true : !is_handle_type(type_id));
	}

	bool ScriptEngine::is_handle_type(int_t type_id)
	{
		return (type_id & asTYPEID_MASK_OBJECT) && (type_id & asTYPEID_OBJHANDLE);
	}

	int_t ScriptEngine::type_id_by_decl(const char* decl)
	{
		return m_engine->GetTypeIdByDecl(decl);
	}

	int_t ScriptEngine::type_id_by_decl(const String& decl)
	{
		return type_id_by_decl(decl.c_str());
	}

	String ScriptEngine::type_declaration(int type_id, bool include_namespace)
	{
		return Strings::make_string(m_engine->GetTypeDeclaration(type_id, include_namespace));
	}

	int_t ScriptEngine::sizeof_primitive_type(int type_id)
	{
		return m_engine->GetSizeOfPrimitiveType(type_id);
	}

	ScriptTypeInfo ScriptEngine::type_info_by_id(int type_id)
	{
		return ScriptTypeInfo(m_engine->GetTypeInfoById(type_id));
	}

	ScriptTypeInfo ScriptEngine::type_info_by_name(const char* name)
	{
		return ScriptTypeInfo(m_engine->GetTypeInfoByName(name));
	}

	ScriptTypeInfo ScriptEngine::type_info_by_decl(const char* decl)
	{
		return ScriptTypeInfo(m_engine->GetTypeInfoByDecl(decl));
	}

	ScriptTypeInfo ScriptEngine::type_info_by_name(const String& name)
	{
		return type_info_by_name(name.c_str());
	}

	ScriptTypeInfo ScriptEngine::type_info_by_decl(const String& decl)
	{
		return type_info_by_decl(decl.c_str());
	}

	String ScriptEngine::variable_name(const void* object, bool include_namespace)
	{
		asIScriptContext* context = asGetActiveContext();

		if (context == nullptr)
		{
			return "";
		}

		std::vector<asIScriptFunction*> functions;

		for (asUINT level = 0, stack_size = context->GetCallstackSize(); level < stack_size; ++level)
		{
			for (asUINT current = 0, props = context->GetVarCount(level); current < props; ++current)
			{
				if (context->GetAddressOfVar(current, level) == object)
				{
					const char* name = nullptr;
					context->GetVar(current, level, &name);

					if (name && !std::string_view(name).empty())
					{
						if (include_namespace)
						{
							if (asIScriptFunction* function = context->GetFunction(level))
							{
								const char* namespace_name = function->GetNamespace();
								if (!std::string_view(namespace_name).empty())
								{
									return Strings::format("{}::{}", namespace_name, name);
								}
							}
						}

						return name;
					}
				}
			}

			functions.push_back(context->GetFunction(level));
		}

		for (auto& func : functions)
		{
			asIScriptModule* module = func->GetModule();
			asUINT count			= module->GetGlobalVarCount();

			for (asUINT prop = 0; prop < count; ++prop)
			{
				if (module->GetAddressOfGlobalVar(prop) == object)
				{
					const char* name		   = nullptr;
					const char* namespace_name = nullptr;
					module->GetGlobalVar(prop, &name, &namespace_name);

					if (name && StringView(name).length() != 0)
					{
						if (include_namespace && namespace_name && !StringView(namespace_name).empty())
						{
							return Strings::format("{}::{}", namespace_name, name);
						}

						return name;
					}
				}
			}
		}

		return "Undefined";
	}

	ScriptEngine& ScriptEngine::register_custom_variable_parser(int_t type_id, VariableToStringFunction function)
	{
		if (function == nullptr)
			return unregister_custom_variable(type_id);
		m_custom_variable_parsers[type_id] = function;
		return instance();
	}

	ScriptEngine& ScriptEngine::unregister_custom_variable(int_t type_id)
	{
		m_custom_variable_parsers.erase(type_id);
		return instance();
	}

	ScriptEngine::VariableToStringFunction ScriptEngine::custom_variable_parser(int_t type_id)
	{
		auto it = m_custom_variable_parsers.find(type_id);
		if (it != m_custom_variable_parsers.end())
			return it->second;
		return nullptr;
	}

	String ScriptEngine::to_string(const byte* address, int_t type_id)
	{
		if (address == nullptr)
			return "null";

		if (auto parser = custom_variable_parser(type_id))
			return parser(address, type_id);

		if (ScriptEngine::is_primitive_type(type_id))
		{
			ScriptVariable var(const_cast<byte*>(address), type_id);

			if (var.is_bool())
				return var.bool_value() ? "true" : "false";
			else if (var.is_int8())
				return Strings::format("{}", var.int8_value());
			else if (var.is_int16())
				return Strings::format("{}", var.int16_value());
			else if (var.is_int32())
				return Strings::format("{}", var.int32_value());
			else if (var.is_int64())
				return Strings::format("{}", var.int64_value());
			else if (var.is_uint8())
				return Strings::format("{}", var.uint8_value());
			else if (var.is_uint16())
				return Strings::format("{}", var.uint16_value());
			else if (var.is_uint32())
				return Strings::format("{}", var.uint32_value());
			else if (var.is_uint64())
				return Strings::format("{}", var.uint64_value());
			else if (var.is_float())
				return Strings::format("{}", var.float_value());
			else if (var.is_double())
				return Strings::format("{}", var.double_value());
		}

		ScriptTypeInfo info = ScriptEngine::type_info_by_id(type_id);
		if (!info.is_valid())
			return Strings::format("{}", reinterpret_cast<const void*>(address));

		if (info.funcdef_signature().is_valid())
		{
			auto func = *reinterpret_cast<const asIScriptFunction* const*>(address);
			if (func == nullptr)
				return "null";
			return func->GetDeclaration(true, true, true);
		}

		auto enum_value_count = info.enum_value_count();

		if (enum_value_count > 0)
		{
			int_t value = *(int_t const*) address;

			for (uint_t i = 0; i < enum_value_count; ++i)
			{
				int_t val;
				StringView text = info.enum_value_by_index(i, &val);

				if (val == value)
				{
					auto ns = info.namespace_name();
					if (!ns.empty())
						return Strings::format("{}::{}::{}", ns, info.name(), text);
					return Strings::format("{}::{}", info.name(), text);
				}
			}
		}

		if (ScriptEngine::is_handle_type(type_id))
		{
			address = *reinterpret_cast<const byte* const*>(address);
		}
		return address ? Strings::format("{}", reinterpret_cast<const void*>(address)) : "null";
	}

	static void variable_name_generic(asIScriptGeneric* generic)
	{
		asUINT arg_type_id	   = generic->GetArgTypeId(0);
		bool is_handle		   = (arg_type_id & asTYPEID_MASK_OBJECT) && (arg_type_id & asTYPEID_OBJHANDLE);
		void* object		   = is_handle ? *reinterpret_cast<void**>(generic->GetArgAddress(0))
										   : reinterpret_cast<void*>(generic->GetArgAddress(0));
		bool include_namespace = static_cast<bool>(generic->GetArgByte(1));
		String name			   = ScriptEngine::variable_name(object, include_namespace);
		generic->SetReturnObject(&name);
	}


	static void on_init()
	{
		ScriptEngine::initialize();
	}

	static void reflection_init()
	{
		ScriptEngine::default_namespace("Engine::ScriptEngine");
		ScriptEngine::register_function("string variable_name(const ?& in variable, bool include_namespace = true)",
										variable_name_generic, ScriptCallConv::Generic);
		ScriptEngine::default_namespace("");
	}

	static PreInitializeController on_preinit([]() { on_init(); }, "Engine::ScriptEngine");
	static ReflectionInitializeController on_reflection_init(reflection_init, "Engine::ScriptEngine");
}// namespace Engine
