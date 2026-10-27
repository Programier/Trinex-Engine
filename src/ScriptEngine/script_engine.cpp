#include <Core/etl/static_array.hpp>
#include <Core/etl/templates.hpp>
#include <Core/stacktrace.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
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

namespace Trinex
{
	class ScriptEngine::State
	{
	public:
		Array<class Script*> scripts;
		asIScriptEngine* engine      = nullptr;
		asIJITCompiler* jit_compiler = nullptr;
		ScriptFolder* script_folder  = nullptr;

		Map<i32, ScriptEngine::VariableToStringFunction> parsers_map;
		Map<i32, Refl::Deprecated::Class*> classes_map;
		ScriptBuildResult* diagnostics_result = nullptr;

	private:
		static void angel_script_callback(const asSMessageInfo* msg, void* param)
		{
			State& data = State::instance();

			if (data.diagnostics_result)
			{
				ScriptDiagnostic diagnostic;
				diagnostic.section = msg->section ? msg->section : "";
				diagnostic.row     = msg->row;
				diagnostic.column  = msg->col;
				diagnostic.message = msg->message ? msg->message : "";

				switch (msg->type)
				{
					case asMSGTYPE_WARNING: diagnostic.type = ScriptDiagnostic::Type::Warning; break;
					case asMSGTYPE_INFORMATION: diagnostic.type = ScriptDiagnostic::Type::Info; break;
					default: diagnostic.type = ScriptDiagnostic::Type::Error; break;
				}

				data.diagnostics_result->diagnostics.push_back(std::move(diagnostic));
			}

			if (msg->type == asMSGTYPE_WARNING)
			{
				trinex_warning(Log::Scripting, "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
			}
			else if (msg->type == asMSGTYPE_INFORMATION)
			{
				trinex_info(Log::Scripting, "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
			}
			else
			{
				if (ScriptEngine::exception_on_error)
				{
					//trinex_(Strings::format("{} ({}, {}): {}", msg->section, msg->row, msg->col, msg->message));
					trinex_error(Log::Scripting, "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
				}
				else
					trinex_error(Log::Scripting, "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
			}
		}

		static void angel_script_translate_exception(asIScriptContext* ctx, void*)
		{
			trinex_unreachable_msg(ctx->GetExceptionString());
		}

		static void* angel_script_allocate(usize size) { return ByteAllocator::allocate(size); }
		static void angel_script_deallocate(void* ptr) { ByteAllocator::deallocate(static_cast<unsigned char*>(ptr)); }

	private:
		State()
		{
			asSetGlobalMemoryFunctions(angel_script_allocate, angel_script_deallocate);

			engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
			trinex_info(Log::Scripting, "Created script engine [%p]", engine);

			engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
			engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, 1);
			engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
			engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);
			engine->SetEngineProperty(asEP_FOREACH_SUPPORT, false);
			engine->SetMessageCallback(asFUNCTION(angel_script_callback), 0, asCALL_CDECL);
			engine->SetTranslateAppExceptionCallback(asFUNCTION(angel_script_translate_exception), nullptr, asCALL_CDECL);

#if ARCH_X86_64 || ARCH_ARM
			if constexpr (enable_jit)
			{
				trinex_info(Log::Scripting, "Enable JIT compiler!");
				jit_compiler = trx_new PlatformJitCompiler();
				engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, true);
				engine->SetJITCompiler(jit_compiler);
			}
#endif
			LifeCycle::on_post_shutdown(ScriptEngine::terminate);

			script_folder = trx_new ScriptFolder("[scripts]:");
		}

		~State()
		{
			if (engine)
			{
				on_terminate();

				trx_delete_inline(script_folder);
				script_folder = nullptr;
				engine->Release();
				engine = nullptr;
			}

			if (jit_compiler)
			{
				trx_delete jit_compiler;
				jit_compiler = nullptr;
			}
		}

	public:
		static State& instance()
		{
			static State state;
			return state;
		}

		asIScriptEngine* operator->() { return engine; }
	};

	bool ScriptEngine::exception_on_error = true;
	CallBacks<void()> ScriptEngine::on_terminate;

	ScriptNamespaceScopedChanger::ScriptNamespaceScopedChanger() : m_prev_namespace(ScriptEngine::default_namespace()) {}

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
		ScriptEngine::default_namespace(m_prev_namespace.c_str());
	}

	void ScriptEngine::terminate() {}

	asIScriptEngine* ScriptEngine::engine()
	{
		return State::instance().engine;
	}

	asIScriptContext* ScriptEngine::new_context()
	{
		return State::instance()->RequestContext();
	}

	static asDWORD create_call_conv(ScriptCallConv conv)
	{
		switch (conv)
		{
			case ScriptCallConv::CDecl: return asCALL_CDECL;
			case ScriptCallConv::StdCall: return asCALL_STDCALL;
			case ScriptCallConv::ThisCallAsGlobal: return asCALL_THISCALL_ASGLOBAL;
			case ScriptCallConv::ThisCall: return asCALL_THISCALL;
			case ScriptCallConv::CDeclObjLast: return asCALL_CDECL_OBJLAST;
			case ScriptCallConv::CDeclObjFirst: return asCALL_CDECL_OBJFIRST;
			case ScriptCallConv::Generic: return asCALL_GENERIC;
			case ScriptCallConv::ThisCall_ObjLast: return asCALL_THISCALL_OBJLAST;
			case ScriptCallConv::ThisCall_ObjFirst: return asCALL_THISCALL_OBJFIRST;
			default: trinex_unreachable_msg("Undefined call convension!"); return asCALL_CDECL;
		}
	}

	ScriptFunction ScriptEngine::register_function(const char* declaration, ScriptFuncPtr* func, ScriptCallConv conv,
	                                               void* auxiliary)
	{
		i32 id = State::instance()->RegisterGlobalFunction(declaration, *reinterpret_cast<asSFuncPtr*>(func),
		                                                   create_call_conv(conv), auxiliary);
		trinex_verify(id >= 0);
		if (id < 0)
			return {};
		return function_by_id(id);
	}

	ScriptFunction ScriptEngine::register_function(const String& declaration, ScriptFuncPtr* func, ScriptCallConv conv,
	                                               void* auxiliary)
	{
		return register_function(declaration.c_str(), func, conv, auxiliary);
	}

	void ScriptEngine::release_context(asIScriptContext* context)
	{
		State::instance()->ReturnContext(context);
	}

	void ScriptEngine::default_namespace(const char* ns)
	{
		State::instance()->SetDefaultNamespace(ns);
	}

	const char* ScriptEngine::default_namespace()
	{
		return State::instance()->GetDefaultNamespace();
	}

	i32 ScriptEngine::register_property(const char* declaration, void* data)
	{
		i32 result = State::instance()->RegisterGlobalProperty(declaration, data);
		trinex_verify(result >= 0);
		return result;
	}

	i32 ScriptEngine::register_property(const String& declaration, void* data)
	{
		return register_property(declaration.c_str(), data);
	}

	class ScriptFolder* ScriptEngine::scripts_folder()
	{
		return State::instance().script_folder;
	}

	void ScriptEngine::begin_diagnostics_capture(ScriptBuildResult* result)
	{
		State::instance().diagnostics_result = result;

		if (result)
		{
			result->success = false;
			result->diagnostics.clear();
		}
	}

	void ScriptEngine::end_diagnostics_capture()
	{
		State::instance().diagnostics_result = nullptr;
	}

	void ScriptEngine::register_class(i32 type_id, Refl::Deprecated::Class* self)
	{
		if (self)
		{
			State::instance().classes_map[type_id] = self;
		}
	}

	Refl::Deprecated::Class* ScriptEngine::find_class(i32 type_id)
	{
		auto& map = State::instance().classes_map;
		auto it   = map.find(type_id);

		if (it == map.end())
			return nullptr;

		return it->second;
	}

	void ScriptEngine::unregister_class(i32 type_id)
	{
		State::instance().classes_map.erase(type_id);
	}

	void ScriptEngine::bind_imports()
	{
		asIScriptEngine* engine = ScriptEngine::engine();

		for (u32 i = 0, j = engine->GetModuleCount(); i < j; i++)
		{
			engine->GetModuleByIndex(i)->BindAllImportedFunctions();
		}
	}

	void ScriptEngine::register_funcdef(const char* declaration)
	{
		trinex_verify(State::instance()->RegisterFuncdef(declaration) >= 0);
	}

	void ScriptEngine::register_funcdef(const String& declaration)
	{
		return register_funcdef(declaration.c_str());
	}

	void ScriptEngine::register_typedef(const char* type, const char* declaration)
	{
		trinex_verify(State::instance()->RegisterTypedef(type, declaration) >= 0);
	}

	void ScriptEngine::register_typedef(const String& type, const String& declaration)
	{
		return register_typedef(type.c_str(), declaration.c_str());
	}

	void ScriptEngine::destroy_script_object(void* object, const ScriptTypeInfo& info)
	{
		State::instance()->ReleaseScriptObject(object, info.info());
	}

	u32 ScriptEngine::global_function_count()
	{
		return State::instance()->GetGlobalFunctionCount();
	}

	ScriptFunction ScriptEngine::global_function_by_index(u32 index)
	{
		return ScriptFunction(State::instance()->GetGlobalFunctionByIndex(index));
	}

	ScriptFunction ScriptEngine::global_function_by_decl(const char* declaration)
	{
		return ScriptFunction(State::instance()->GetGlobalFunctionByDecl(declaration));
	}

	ScriptFunction ScriptEngine::global_function_by_decl(const String& declaration)
	{
		return global_function_by_decl(declaration.c_str());
	}

	u32 ScriptEngine::global_property_count()
	{
		return State::instance()->GetGlobalPropertyCount();
	}

	i32 ScriptEngine::global_property_index_by_name(const char* name)
	{
		return State::instance()->GetGlobalPropertyIndexByName(name);
	}

	i32 ScriptEngine::global_property_index_by_name(const String& name)
	{
		return global_property_index_by_name(name.c_str());
	}

	i32 ScriptEngine::global_property_index_by_decl(const char* declaration)
	{
		return State::instance()->GetGlobalPropertyIndexByDecl(declaration);
	}

	i32 ScriptEngine::global_property_index_by_decl(const String& declaration)
	{
		return global_property_index_by_decl(declaration.c_str());
	}

	bool ScriptEngine::global_property(u32 index, StringView* name, StringView* name_space, StringView* config_group,
	                                   i32* type_id, bool* is_const, u8** pointer)
	{
		const char* c_name       = nullptr;
		const char* c_name_space = nullptr;
		const char* c_group      = nullptr;

		bool result = State::instance()->GetGlobalPropertyByIndex(
		                      index, name ? &c_name : nullptr, name_space ? &c_name_space : nullptr, type_id, is_const,
		                      config_group ? &c_group : nullptr, reinterpret_cast<void**>(pointer))
		              >= 0;

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

			if (config_group)
			{
				*config_group = c_group ? c_group : "";
			}
		}

		return result;
	}

	bool ScriptEngine::begin_config_group(const char* group)
	{
		return State::instance()->BeginConfigGroup(group) >= 0;
	}

	bool ScriptEngine::begin_config_group(const String& group)
	{
		return begin_config_group(group.c_str());
	}

	bool ScriptEngine::end_config_group()
	{
		return State::instance()->EndConfigGroup() >= 0;
	}

	bool ScriptEngine::remove_config_group(const char* group)
	{
		return State::instance()->RemoveConfigGroup(group) >= 0;
	}

	bool ScriptEngine::remove_config_group(const String& group)
	{
		return remove_config_group(group.c_str());
	}

	void ScriptEngine::garbage_collect(GarbageCollectFlags flags, usize iterations)
	{
		State::instance()->GarbageCollect(flags, iterations);
	}

	u32 ScriptEngine::object_type_count()
	{
		return State::instance()->GetObjectTypeCount();
	}

	ScriptTypeInfo ScriptEngine::object_type_by_index(u32 index)
	{
		return ScriptTypeInfo(State::instance()->GetObjectTypeByIndex(index));
	}

	bool ScriptEngine::exec_string(const String& line)
	{
		return exec_string(line.c_str());
	}

	bool ScriptEngine::exec_string(const char* line)
	{
		return ExecuteString(State::instance().engine, line) >= 0;
	}

	// Enums
	u32 ScriptEngine::enum_count()
	{
		return State::instance()->GetEnumCount();
	}

	ScriptTypeInfo ScriptEngine::enum_by_index(u32 index)
	{
		return ScriptTypeInfo(State::instance()->GetEnumByIndex(index));
	}

	// Funcdefs
	u32 ScriptEngine::funcdef_count()
	{
		return State::instance()->GetFuncdefCount();
	}

	ScriptTypeInfo ScriptEngine::funcdef_by_index(u32 index)
	{
		return ScriptTypeInfo(State::instance()->GetFuncdefByIndex(index));
	}

	// Typedefs
	u32 ScriptEngine::typedef_count()
	{
		return State::instance()->GetTypedefCount();
	}

	ScriptTypeInfo ScriptEngine::typedef_by_index(u32 index)
	{
		return ScriptTypeInfo(State::instance()->GetTypedefByIndex(index));
	}

	// Script modules
	void ScriptEngine::discard_module(const char* module_name)
	{
		State::instance()->DiscardModule(module_name);
	}

	void ScriptEngine::discard_module(const String& module_name)
	{
		return discard_module(module_name.c_str());
	}

	u32 ScriptEngine::module_count()
	{
		return State::instance()->GetModuleCount();
	}

	ScriptModule ScriptEngine::module_by_index(u32 index)
	{
		return ScriptModule(State::instance()->GetModuleByIndex(index));
	}

	ScriptModule ScriptEngine::module_by_name(const char* name, ScriptModuleLookup lookup)
	{
		asEGMFlags flags = asGM_ONLY_IF_EXISTS;

		switch (lookup)
		{
			case ScriptModuleLookup::OnlyIfExists: flags = asGM_ONLY_IF_EXISTS; break;
			case ScriptModuleLookup::AlwaysCreate: flags = asGM_ALWAYS_CREATE; break;
			case ScriptModuleLookup::CreateIfNotExists: flags = asGM_CREATE_IF_NOT_EXISTS; break;
		}

		return ScriptModule(State::instance()->GetModule(name, flags));
	}

	// Script functions
	i32 ScriptEngine::last_function_id()
	{
		return State::instance()->GetLastFunctionId();
	}

	ScriptFunction ScriptEngine::function_by_id(i32 func_id)
	{
		return ScriptFunction(State::instance()->GetFunctionById(func_id));
	}

	// Type identification

	bool ScriptEngine::is_primitive_type(i32 type_id)
	{
		return sizeof_primitive_type(type_id) != 0;
	}

	bool ScriptEngine::is_bool(i32 type_id)
	{
		return type_id == asTYPEID_BOOL;
	}

	bool ScriptEngine::is_int8(i32 type_id)
	{
		return type_id == asTYPEID_INT8;
	}

	bool ScriptEngine::is_int16(i32 type_id)
	{
		return type_id == asTYPEID_INT16;
	}

	bool ScriptEngine::is_int32(i32 type_id)
	{
		return type_id == asTYPEID_INT32;
	}

	bool ScriptEngine::is_int64(i32 type_id)
	{
		return type_id == asTYPEID_INT64;
	}

	bool ScriptEngine::is_uint8(i32 type_id)
	{
		return type_id == asTYPEID_UINT8;
	}

	bool ScriptEngine::is_uint16(i32 type_id)
	{
		return type_id == asTYPEID_UINT16;
	}

	bool ScriptEngine::is_uint32(i32 type_id)
	{
		return type_id == asTYPEID_UINT32;
	}

	bool ScriptEngine::is_uint64(i32 type_id)
	{
		return type_id == asTYPEID_UINT64;
	}

	bool ScriptEngine::is_float(i32 type_id)
	{
		return type_id == asTYPEID_FLOAT;
	}

	bool ScriptEngine::is_double(i32 type_id)
	{
		return type_id == asTYPEID_DOUBLE;
	}

	bool ScriptEngine::is_object_type(i32 type_id, bool handle_is_object)
	{
		return (type_id & asTYPEID_MASK_OBJECT) && (handle_is_object ? true : !is_handle_type(type_id));
	}

	bool ScriptEngine::is_handle_type(i32 type_id)
	{
		return (type_id & asTYPEID_MASK_OBJECT) && (type_id & asTYPEID_OBJHANDLE);
	}

	i32 ScriptEngine::type_id_by_decl(const char* decl)
	{
		return State::instance()->GetTypeIdByDecl(decl);
	}

	i32 ScriptEngine::type_id_by_decl(const String& decl)
	{
		return type_id_by_decl(decl.c_str());
	}

	String ScriptEngine::type_declaration(int type_id, bool include_namespace)
	{
		return Strings::make_string(State::instance()->GetTypeDeclaration(type_id, include_namespace));
	}

	i32 ScriptEngine::sizeof_primitive_type(int type_id)
	{
		return State::instance()->GetSizeOfPrimitiveType(type_id);
	}

	ScriptTypeInfo ScriptEngine::type_info_by_id(int type_id)
	{
		return ScriptTypeInfo(State::instance()->GetTypeInfoById(type_id));
	}

	ScriptTypeInfo ScriptEngine::type_info_by_name(const char* name)
	{
		return ScriptTypeInfo(State::instance()->GetTypeInfoByName(name));
	}

	ScriptTypeInfo ScriptEngine::type_info_by_decl(const char* decl)
	{
		return ScriptTypeInfo(State::instance()->GetTypeInfoByDecl(decl));
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
			asUINT count            = module->GetGlobalVarCount();

			for (asUINT prop = 0; prop < count; ++prop)
			{
				if (module->GetAddressOfGlobalVar(prop) == object)
				{
					const char* name           = nullptr;
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

	void ScriptEngine::register_custom_variable_parser(i32 type_id, VariableToStringFunction function)
	{
		if (function == nullptr)
			return unregister_custom_variable(type_id);

		State::instance().parsers_map[type_id] = function;
	}

	void ScriptEngine::unregister_custom_variable(i32 type_id)
	{
		State::instance().parsers_map.erase(type_id);
	}

	ScriptEngine::VariableToStringFunction ScriptEngine::custom_variable_parser(i32 type_id)
	{
		State& state = State::instance();

		auto it = state.parsers_map.find(type_id);
		if (it != state.parsers_map.end())
			return it->second;

		return nullptr;
	}

	static String script_object_to_string_ref(const u8* object, asIScriptFunction* func)
	{
		String* result = nullptr;
		ScriptContext::current()->execute(object, ScriptFunction(func), &result);
		return *result;
	}

	static String script_object_to_string_value(const u8* object, asIScriptFunction* func)
	{
		String result;
		ScriptContext::current()->execute(object, ScriptFunction(func), &result);
		return result;
	}

	template<bool repr>
	static StaticArray<const char*, 6> static_object_to_string_operator_names()
	{
		return {"const string& opRepr() const", "string& opRepr() const", "string opRepr() const", nullptr, nullptr, nullptr};
	}

	template<>
	StaticArray<const char*, 6> static_object_to_string_operator_names<false>()
	{
		return {"const string& opConv() const",     "string& opConv() const",     "string opConv() const",
		        "const string& opImplConv() const", "string& opImplConv() const", "string opImplConv() const"};
	}

	template<bool repr>
	static const Pair<String (*)(const u8*, asIScriptFunction*), asIScriptFunction*>
	static_find_string_op(const ScriptTypeInfo& info)
	{
		static Map<i32, Pair<String (*)(const u8*, asIScriptFunction*), asIScriptFunction*>> func_map;
		auto it = func_map.find(info.type_id());

		if (it != func_map.end())
			return it->second;

		StaticArray<const char*, 6> funcs = static_object_to_string_operator_names<repr>();

		for (const char* name : funcs)
		{
			if (name == nullptr)
				break;

			auto func = info.method_by_decl(name);

			if (!func.is_valid())
				continue;

			ScriptTypeModifiers modifiers;
			if (func.return_type_id(&modifiers) < 0)
				continue;

			if (!info.is_native())
			{
				auto module = info.module();
				if (Script* script = ScriptEngine::scripts_folder()->find_script(Path(module.name())))
					script->on_discard.push([id = info.type_id()](Script*) { func_map.erase(id); });
			}

			if (modifiers & ScriptTypeModifiers::OutRef)
			{
				auto ref = func_map.insert({info.type_id(), {&script_object_to_string_ref, func.function()}});
				return ref.first->second;
			}
			else
			{
				auto ref = func_map.insert({info.type_id(), {&script_object_to_string_value, func.function()}});
				return ref.first->second;
			}
		}

		func_map.insert({info.type_id(), {nullptr, nullptr}});
		return {nullptr, nullptr};
	}

	String ScriptEngine::to_string(const u8* address, i32 type_id, bool repr)
	{
		if (address == nullptr)
			return "null";

		if (auto parser = custom_variable_parser(type_id))
			return parser(address, type_id, repr);

		// if (ScriptEngine::is_primitive_type(type_id))
		// {
		// 	ScriptVariable var(const_cast<u8*>(address), type_id);

		// 	if (var.is_bool())
		// 		return var.bool_value() ? "true" : "false";
		// 	else if (var.is_int8())
		// 		return Strings::format("{}", var.int8_value());
		// 	else if (var.is_int16())
		// 		return Strings::format("{}", var.int16_value());
		// 	else if (var.is_int32())
		// 		return Strings::format("{}", var.int32_value());
		// 	else if (var.is_int64())
		// 		return Strings::format("{}", var.int64_value());
		// 	else if (var.is_uint8())
		// 		return Strings::format("{}", var.uint8_value());
		// 	else if (var.is_uint16())
		// 		return Strings::format("{}", var.uint16_value());
		// 	else if (var.is_uint32())
		// 		return Strings::format("{}", var.uint32_value());
		// 	else if (var.is_uint64())
		// 		return Strings::format("{}", var.uint64_value());
		// 	else if (var.is_float())
		// 		return Strings::format("{}", var.float_value());
		// 	else if (var.is_double())
		// 		return Strings::format("{}", var.double_value());
		// }

		ScriptTypeInfo info = ScriptEngine::type_info_by_id(type_id);
		if (!info.is_valid())
			return Strings::format("<undefined object at {}>", reinterpret_cast<const void*>(address));

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
			i32 value = *(i32 const*) address;

			for (u32 i = 0; i < enum_value_count; ++i)
			{
				i64 val;
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
			address = *reinterpret_cast<const u8* const*>(address);
		}

		if (address)
		{
			auto op = repr ? static_find_string_op<true>(info) : static_find_string_op<false>(info);
			if (op.first && op.second)
			{
				return op.first(address, op.second);
			}
		}

		String str_address = address ? Strings::format("{}", static_cast<const void*>(address)) : "null";
		if (repr)
			return str_address;

		String full_class_name = Strings::concat_scoped_name(info.namespace_name(), info.name());
		return Strings::format("<{} at {}>", full_class_name, str_address);
	}

	bool ScriptEngine::assign_script_object(void* dst, void* src, const ScriptTypeInfo& info)
	{
		return State::instance()->AssignScriptObject(dst, src, info.info()) >= 0;
	}

	static void variable_name_generic(asIScriptGeneric* generic)
	{
		asUINT arg_type_id     = generic->GetArgTypeId(0);
		bool is_handle         = (arg_type_id & asTYPEID_MASK_OBJECT) && (arg_type_id & asTYPEID_OBJHANDLE);
		void* object           = is_handle ? *reinterpret_cast<void**>(generic->GetArgAddress(0))
		                                   : reinterpret_cast<void*>(generic->GetArgAddress(0));
		bool include_namespace = static_cast<bool>(generic->GetArgByte(1));
		String name            = ScriptEngine::variable_name(object, include_namespace);
		generic->SetReturnObject(&name);
	}

	trinex_on_reflection_init()
	{
		ScriptEngine::default_namespace("Trinex::ScriptEngine");
		ScriptEngine::register_function("string variable_name(const ?& variable, bool include_namespace = true)",
		                                variable_name_generic, ScriptCallConv::Generic);
		ScriptEngine::default_namespace("");
	}
} // namespace Trinex
