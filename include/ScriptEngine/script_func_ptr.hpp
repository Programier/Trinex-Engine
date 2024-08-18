#pragma once

#include <Core/export.hpp>

class asIScriptGeneric;

namespace Engine
{
	struct ENGINE_EXPORT ScriptFuncPtr {
	public:
		using GlobalFunction  = void (*)();
		using GenericFunction = void (*)(asIScriptGeneric*);

	private:
		static ScriptFuncPtr* function_ptr_generic(GenericFunction func);
		static ScriptFuncPtr* function_ptr_global(GlobalFunction func);

	public:
		template<typename ReturnValue, typename... Args>
		static inline ScriptFuncPtr* function_ptr(ReturnValue (*func)(Args...))
		{
			return function_ptr_global(reinterpret_cast<GlobalFunction>(func));
		}
	};

	template<>
	inline ScriptFuncPtr* ScriptFuncPtr::function_ptr<void, asIScriptGeneric*>(GenericFunction function)
	{
		return function_ptr_generic(function);
	}

	struct ENGINE_EXPORT ScriptMethodPtr {
	public:
		using GeneralMethod = void (ScriptMethodPtr::*)();

	private:
		static ScriptMethodPtr* method_ptr_general(GeneralMethod function);

	public:
		template<typename ReturnValue, typename Instance, typename... Args>
		static inline ScriptMethodPtr* method_ptr(ReturnValue (Instance::*func)(Args...))
		{
			return method_ptr_general(reinterpret_cast<GeneralMethod>(func));
		}

		template<typename ReturnValue, typename Instance, typename... Args>
		static inline ScriptMethodPtr* method_ptr(ReturnValue (Instance::*func)(Args...) const)
		{
			return method_ptr_general(reinterpret_cast<GeneralMethod>(func));
		}
	};
}// namespace Engine
