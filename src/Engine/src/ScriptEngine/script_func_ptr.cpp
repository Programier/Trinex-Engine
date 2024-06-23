#include <ScriptEngine/script_func_ptr.hpp>
#include <angelscript.h>


namespace Engine
{
    ScriptFuncPtr* ScriptFuncPtr::function_ptr_generic(GenericFunction function)
    {
        static thread_local asSFuncPtr ptr;
        ptr = asFunctionPtr(function);
        return reinterpret_cast<ScriptFuncPtr*>(&ptr);
    }

    ScriptFuncPtr* ScriptFuncPtr::function_ptr_global(GlobalFunction function)
    {
        static thread_local asSFuncPtr ptr;
        ptr = asFunctionPtr(function);
        return reinterpret_cast<ScriptFuncPtr*>(&ptr);
    }

    ScriptMethodPtr* ScriptMethodPtr::method_ptr_general(GeneralMethod function)
    {
        static thread_local asSFuncPtr ptr;
        ptr = asSMethodPtr<sizeof(function)>::Convert(function);
        return reinterpret_cast<ScriptMethodPtr*>(&ptr);
    }
}// namespace Engine
