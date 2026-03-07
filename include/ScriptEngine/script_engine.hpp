#pragma once
#include <Core/callback.hpp>
#include <Core/etl/map.hpp>
#include <ScriptEngine/enums.hpp>
#include <ScriptEngine/script_func_ptr.hpp>
#include <ScriptEngine/script_function.hpp>

class asIScriptEngine;
class asIScriptContext;
class asIJITCompiler;
class asIScriptGeneric;

namespace Trinex
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
		using VariableToStringFunction = String (*)(const u8* object, i32 type_id, bool repr);

	private:
		static void terminate();
		static asIScriptContext* new_context();
		static ScriptEngine& destroy_script_object(void*, const ScriptTypeInfo& info);

	public:
		enum GarbageCollectFlags
		{
			FullCycle      = 1,
			OneStep        = 2,
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
		static i32 register_property(const char* declaration, void* data);
		static i32 register_property(const String& declaration, void* data);
		static class ScriptFolder* scripts_folder();
		static ScriptEngine& load_scripts();

		static ScriptEngine& register_class(i32 type_id, Refl::Class* self);
		static Refl::Class* find_class(i32 type_id);
		static ScriptEngine& unregister_class(i32 type_id);

		static ScriptEngine& bind_imports();
		static ScriptEngine& register_funcdef(const char* declaration);
		static ScriptEngine& register_funcdef(const String& declaration);
		static ScriptEngine& register_typedef(const char* new_type_name, const char* type);
		static ScriptEngine& register_typedef(const String& new_type_name, const String& type);

		static u32 global_function_count();
		static ScriptFunction global_function_by_index(u32 index);
		static ScriptFunction global_function_by_decl(const char* declaration);
		static ScriptFunction global_function_by_decl(const String& declaration);

		static u32 global_property_count();
		static i32 global_property_index_by_name(const char* name);
		static i32 global_property_index_by_name(const String& name);
		static i32 global_property_index_by_decl(const char* declaration);
		static i32 global_property_index_by_decl(const String& declaration);
		static bool global_property(u32 index, StringView* name = nullptr, StringView* name_space = nullptr,
		                            StringView* config_group = nullptr, i32* type_id = nullptr, bool* is_const = nullptr,
		                            u8** pointer = nullptr);

		static bool begin_config_group(const char* group);
		static bool begin_config_group(const String& group);
		static bool end_config_group();
		static bool remove_config_group(const char* group);
		static bool remove_config_group(const String& group);

		static ScriptEngine& garbage_collect(BitMask flags = GarbageCollectFlags::FullCycle, usize iterations = 1);

		static u32 object_type_count();
		static ScriptTypeInfo object_type_by_index(u32 index);

		static bool exec_string(const String& line);
		static bool exec_string(const char* line);

		// Enums
		static u32 enum_count();
		static ScriptTypeInfo enum_by_index(u32 index);

		// Funcdefs
		static u32 funcdef_count();
		static ScriptTypeInfo funcdef_by_index(u32 index);

		// Typedefs
		static u32 typedef_count();
		static ScriptTypeInfo typedef_by_index(u32 index);

		// Script modules
		static ScriptEngine& discard_module(const char* module);
		static ScriptEngine& discard_module(const String& module);
		static u32 module_count();
		static ScriptModule module_by_index(u32 index);
		static ScriptModule module_by_name(const char* name, ScriptModuleLookup lookup = ScriptModuleLookup::OnlyIfExists);

		// Script functions
		static i32 last_function_id();
		static ScriptFunction function_by_id(i32 func_id);

		// Type identification
		static bool is_primitive_type(i32 type_id);
		static bool is_bool(i32 type_id);
		static bool is_int8(i32 type_id);
		static bool is_int16(i32 type_id);
		static bool is_int32(i32 type_id);
		static bool is_int64(i32 type_id);
		static bool is_uint8(i32 type_id);
		static bool is_uint16(i32 type_id);
		static bool is_uint32(i32 type_id);
		static bool is_uint64(i32 type_id);
		static bool is_float(i32 type_id);
		static bool is_double(i32 type_id);
		static bool is_object_type(i32 type_id, bool handle_is_object = false);
		static bool is_handle_type(i32 type_id);

		static i32 type_id_by_decl(const char* decl);
		static i32 type_id_by_decl(const String& decl);
		static String type_declaration(i32 type_id, bool include_namespace = false);
		static i32 sizeof_primitive_type(i32 type_id);
		static ScriptTypeInfo type_info_by_id(i32 type_id);
		static ScriptTypeInfo type_info_by_name(const char* name);
		static ScriptTypeInfo type_info_by_decl(const char* decl);
		static ScriptTypeInfo type_info_by_name(const String& name);
		static ScriptTypeInfo type_info_by_decl(const String& decl);

		// User functions
		static String variable_name(const void* object, bool include_namespace = true);

		// Functions register

		static ScriptFunction register_function(const char* declaration, ScriptFuncPtr* func,
		                                        ScriptCallConv conv = ScriptCallConv::CDecl, void* auxiliary = nullptr);
		static ScriptFunction register_function(const String& declaration, ScriptFuncPtr* func,
		                                        ScriptCallConv conv = ScriptCallConv::CDecl, void* auxiliary = nullptr);

		// Variable to string
		static ScriptEngine& register_custom_variable_parser(i32 type_id, VariableToStringFunction function);
		static ScriptEngine& unregister_custom_variable(i32 type_id);
		static VariableToStringFunction custom_variable_parser(i32 type_id);
		static String to_string(const u8* object, i32 type_id, bool repr = false);

		// Assign
		static bool assign_script_object(void* dst, void* src, const ScriptTypeInfo& info);

		// Templates

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
}// namespace Trinex
