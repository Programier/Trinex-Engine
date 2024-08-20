#pragma once
#include <Core/callback.hpp>
#include <Core/enums.hpp>
#include <ScriptEngine/script_func_ptr.hpp>
#include <ScriptEngine/script_function.hpp>

class asIScriptEngine;
class asIScriptContext;
class asIJITCompiler;
class asIScriptGeneric;

namespace Engine
{
	class ScriptModule;
	class ScriptTypeInfo;
	class ScriptObject;
	class ScriptFunction;

	class ENGINE_EXPORT ScriptNamespaceScopedChanger final
	{
		String m_prev_namespace;

	public:
		ScriptNamespaceScopedChanger();
		ScriptNamespaceScopedChanger(const char* new_namespace);
		ScriptNamespaceScopedChanger(const String& new_namespace);
		const String& saved_namespace() const;
		~ScriptNamespaceScopedChanger();
	};

	class ENGINE_EXPORT ScriptEngine
	{
	public:
		using VariableToStringFunction = String (*)(const byte* object, int_t type_id);

	private:
		static Vector<class Script*> m_scripts;
		static asIScriptEngine* m_engine;
		static asIJITCompiler* m_jit_compiler;
		static class ScriptFolder* m_script_folder;
		static TreeMap<int_t, VariableToStringFunction> m_custom_variable_parsers;

		static void terminate();
		static asIScriptContext* new_context();
		static ScriptEngine& destroy_script_object(ScriptObjectAddress, const ScriptTypeInfo& info);

	public:
		enum GarbageCollectFlags
		{
			FullCycle	   = 1,
			OneStep		   = 2,
			DestroyGarbage = 4,
			DetectGarbage  = 8
		};

		static bool exception_on_error;
		static CallBacks<void()> on_terminate;

		static ScriptEngine& initialize();
		static ScriptEngine& instance();
		static asIScriptEngine* engine();
		static const ScriptEngine& release_context(asIScriptContext* context);

		static ScriptEngine& default_namespace(const String& name);
		static ScriptEngine& default_namespace(const char* ns);
		static StringView default_namespace();
		static int_t register_property(const char* declaration, void* data);
		static int_t register_property(const String& declaration, void* data);
		static ScriptModule create_module(const String& name, EnumerateType flags = 0);
		static ScriptModule create_module(const char* name, EnumerateType flags = 0);
		static uint_t module_count();
		static class ScriptFolder* scripts_folder();
		static ScriptEngine& load_scripts();

		static ScriptEngine& bind_imports();
		static ScriptEngine& funcdef(const char* declaration);
		static ScriptEngine& funcdef(const String& declaration);
		static ScriptEngine& register_typedef(const char* new_type_name, const char* type);
		static ScriptEngine& register_typedef(const String& new_type_name, const String& type);

		static uint_t global_function_count();
		static ScriptFunction global_function_by_index(uint_t index);
		static ScriptFunction global_function_by_decl(const char* declaration);
		static ScriptFunction global_function_by_decl(const String& declaration);

		static uint_t global_property_count();
		static int_t global_property_index_by_name(const char* name);
		static int_t global_property_index_by_name(const String& name);
		static int_t global_property_index_by_decl(const char* declaration);
		static int_t global_property_index_by_decl(const String& declaration);
		static bool global_property(uint_t index, StringView* name = nullptr, StringView* name_space = nullptr,
									int_t* type_id = nullptr, bool* is_const = nullptr, byte** pointer = nullptr);

		static ScriptEngine& garbage_collect(BitMask flags = GarbageCollectFlags::FullCycle, size_t iterations = 1);

		static uint_t object_type_count();
		static ScriptTypeInfo object_type_by_index(uint_t index);

		static bool exec_string(const String& line);
		static bool exec_string(const char* line);

		// Enums
		static uint_t enum_count();
		static ScriptTypeInfo enum_by_index(uint_t index);

		// Funcdefs
		static uint_t funcdef_count();
		static ScriptTypeInfo funcdef_by_index(uint_t index);

		// Typedefs
		static uint_t typedef_count();
		static ScriptTypeInfo typedef_by_index(uint_t index);

		// Script modules
		static ScriptEngine& discard_module(const char* module);
		static ScriptEngine& discard_module(const String& module);
		static ScriptModule module_by_index(uint_t index);

		// Script functions
		static int_t last_function_id();
		static ScriptFunction function_by_id(int_t func_id);

		// Type identification
		static bool is_primitive_type(int_t type_id);
		static bool is_bool(int_t type_id);
		static bool is_int8(int_t type_id);
		static bool is_int16(int_t type_id);
		static bool is_int32(int_t type_id);
		static bool is_int64(int_t type_id);
		static bool is_uint8(int_t type_id);
		static bool is_uint16(int_t type_id);
		static bool is_uint32(int_t type_id);
		static bool is_uint64(int_t type_id);
		static bool is_float(int_t type_id);
		static bool is_double(int_t type_id);
		static bool is_object_type(int_t type_id, bool handle_is_object = false);
		static bool is_handle_type(int_t type_id);

		static int_t type_id_by_decl(const char* decl);
		static int_t type_id_by_decl(const String& decl);
		static String type_declaration(int_t type_id, bool include_namespace = false);
		static int_t sizeof_primitive_type(int_t type_id);
		static ScriptTypeInfo type_info_by_id(int_t type_id);
		static ScriptTypeInfo type_info_by_name(const char* name);
		static ScriptTypeInfo type_info_by_decl(const char* decl);
		static ScriptTypeInfo type_info_by_name(const String& name);
		static ScriptTypeInfo type_info_by_decl(const String& decl);

		// User functions
		static String variable_name(const void* object, bool include_namespace = true);

		// Functions register

		static ScriptFunction register_function(const char* declaration, ScriptFuncPtr* func,
												ScriptCallConv conv = ScriptCallConv::CDecl);
		static ScriptFunction register_function(const String& declaration, ScriptFuncPtr* func,
												ScriptCallConv conv = ScriptCallConv::CDecl);

		// Variable to string
		static ScriptEngine& register_custom_variable_parser(int_t type_id, VariableToStringFunction function);
		static ScriptEngine& unregister_custom_variable(int_t type_id);
		static VariableToStringFunction custom_variable_parser(int_t type_id);
		static String to_string(const byte* object, int_t type_id);

		template<typename ReturnValue, typename... Args>
		static ScriptFunction register_function(const char* declaration, ReturnValue (*func)(Args...),
												ScriptCallConv conv = ScriptCallConv::CDecl)
		{
			return register_function(declaration, ScriptFuncPtr::function_ptr(func), conv);
		}

		template<typename ReturnValue, typename... Args>
		static ScriptFunction register_function(const String& declaration, ReturnValue (*func)(Args...),
												ScriptCallConv conv = ScriptCallConv::CDecl)
		{
			return register_function(declaration.c_str(), func, conv);
		}

		friend class ScriptFunction;
		friend class ScriptClassRegistrar;
		friend class ScriptObject;
	};
}// namespace Engine
