#include <Core/engine_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

#if ARCH_ARM
#include "jit_compiler/arm64/compiler.hpp"
using PlatformJitCompiler = JIT::ARM64_Compiler;

#elif ARCH_X86_64
#include "jit_compiler/x86-64/compiler.hpp"
using PlatformJitCompiler = JIT::X86_64_Compiler;
#endif


namespace Print
{
    void asRegister(asIScriptEngine* engine);
}

namespace Engine
{
    namespace Initializers
    {
        void init_primitive_wrappers();
    }// namespace Initializers

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
            //throw EngineException(Strings::format("{} ({}, {}): {}", msg->section, msg->row, msg->col, msg->message));
            error_log("ScriptEngine", "%s (%d, %d): %s", msg->section, msg->row, msg->col, msg->message);
        }
    }


    struct ScriptContextManager {
        Set<asIScriptContext*> _M_context_array;
        Vector<asIScriptContext*> _M_free_context_array;

        asIScriptContext* new_context()
        {
            if (_M_free_context_array.empty())
            {
                asIScriptContext* context = ScriptEngine::instance()->as_engine()->CreateContext();
                debug_log("ScriptContextManager", "Created context '%p'. Count of contexts: %zu", context,
                          _M_context_array.size() + 1);
                _M_context_array.insert(context);
                return context;
            }
            asIScriptContext* context = _M_free_context_array.back();
            _M_free_context_array.pop_back();
            return context;
        }

        ScriptContextManager& free_context(asIScriptContext* context)
        {
            _M_free_context_array.push_back(context);
            return *this;
        }

        ScriptContextManager& release_context(asIScriptContext* context, bool full_remove)
        {
            if (full_remove)
            {
                _M_context_array.erase(context);
            }

            debug_log("ScriptContextManager", "Released context '%p'", context);
            context->Release();
            return *this;
        }

        ScriptContextManager& cleanup()
        {
            for (asIScriptContext* context : _M_free_context_array)
            {
                release_context(context, true);
            }

            _M_free_context_array.clear();

            return *this;
        }

        ~ScriptContextManager()
        {
            for (asIScriptContext* context : _M_context_array)
            {
                release_context(context, false);
            }

            _M_context_array.clear();
            _M_free_context_array.clear();
        }
    };


    ScriptEngine* ScriptEngine::_M_instance = nullptr;

    ScriptEngine::ScriptEngine()
    {
        _M_instance = this;

        logger->log("ScriptEngine", "Created script engine [%p]", this);
        _M_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

        _M_engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
        _M_engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, 1);
        _M_engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
        _M_engine->SetMessageCallback(asFUNCTION(angel_script_callback), 0, asCALL_CDECL);

#if ARCH_X86_64 || ARCH_ARM
        if (engine_config.enable_jit)
        {
            info_log("ScriptEngine", "Enable JIT compiler!");
            auto compiler   = new PlatformJitCompiler();
            _M_jit_compiler = compiler;
            _M_engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, true);
            _M_engine->SetJITCompiler(_M_jit_compiler);

#if TRINEX_WITH_SKIP_JIT_INSTRUCTIONS
            for (auto& [func_name, indices] : engine_config.jit_skip_instructions)
            {
                for (auto index : indices)
                {
                    compiler->push_instruction_index_for_skip(func_name, index);
                }
            }
#endif
        }
#endif

        asInitializeAddons(_M_engine);
        _M_context_manager = new ScriptContextManager();


        Print::asRegister(_M_engine);
        Initializers::init_primitive_wrappers();
    }

    ScriptEngine::~ScriptEngine()
    {
        delete _M_context_manager;
        _M_engine->Release();
        release_scripts();
        if (_M_jit_compiler)
        {
            delete _M_jit_compiler;
        }
    }

    void ScriptEngine::initialize()
    {
        if (_M_instance == nullptr)
        {
            new ScriptEngine();
            PostDestroyController controller(ScriptEngine::terminate);
        }
    }

    void ScriptEngine::terminate()
    {
        if (_M_instance)
        {
            delete _M_instance;
            _M_instance = nullptr;
        }
    }

    ScriptEngine* ScriptEngine::instance()
    {
        return _M_instance;
    }

    asIScriptEngine* ScriptEngine::as_engine() const
    {
        return _M_engine;
    }

    asIScriptContext* ScriptEngine::new_context() const
    {
        return _M_context_manager->new_context();
    }

    static asDWORD create_call_conv(ScriptCallConv conv)
    {
        switch (conv)
        {
            case ScriptCallConv::CDECL:
                return asCALL_CDECL;
            case ScriptCallConv::STDCALL:
                return asCALL_STDCALL;
            case ScriptCallConv::THISCALL_ASGLOBAL:
                return asCALL_THISCALL_ASGLOBAL;
            case ScriptCallConv::THISCALL:
                return asCALL_THISCALL;
            case ScriptCallConv::CDECL_OBJLAST:
                return asCALL_CDECL_OBJLAST;
            case ScriptCallConv::CDECL_OBJFIRST:
                return asCALL_CDECL_OBJFIRST;
            case ScriptCallConv::GENERIC:
                return asCALL_GENERIC;
            case ScriptCallConv::THISCALL_OBJLAST:
                return asCALL_THISCALL_OBJLAST;
            case ScriptCallConv::THISCALL_OBJFIRST:
                return asCALL_THISCALL_OBJFIRST;
            default:
                throw EngineException("Undefined call convension!");
        }
    }

    ScriptEngine& ScriptEngine::private_register_function(const char* declaration, void* func, ScriptCallConv conv)
    {
        _M_engine->RegisterGlobalFunction(declaration, asFunctionPtr(func), create_call_conv(conv));
        return *this;
    }

    const ScriptEngine& ScriptEngine::release_context(asIScriptContext* context) const
    {
        _M_context_manager->free_context(context);
        return *this;
    }

    const ScriptEngine& ScriptEngine::cleanup_free_contexts() const
    {
        _M_context_manager->cleanup();
        return *this;
    }

    ScriptEngine& ScriptEngine::default_namespace(const String& name)
    {
        return default_namespace(name.c_str());
    }

    ScriptEngine& ScriptEngine::default_namespace(const char* ns)
    {
        _M_engine->SetDefaultNamespace(ns);
        return *this;
    }

    String ScriptEngine::default_namespace() const
    {
        return _M_engine->GetDefaultNamespace();
    }

    ScriptEngine& ScriptEngine::register_property(const char* declaration, void* data)
    {
        _M_engine->RegisterGlobalProperty(declaration, data);
        return *this;
    }

    ScriptEngine& ScriptEngine::register_property(const String& declaration, void* data)
    {
        return register_property(declaration.c_str(), data);
    }


    ScriptModule ScriptEngine::global_module() const
    {
        return _M_engine->GetModule("Global", asGM_CREATE_IF_NOT_EXISTS);
    }

    ScriptModule ScriptEngine::module(uint_t index)
    {
        return _M_engine->GetModuleByIndex(index);
    }

    uint_t ScriptEngine::module_count() const
    {
        return _M_engine->GetModuleCount();
    }

    ScriptEngine& ScriptEngine::bind_imports()
    {
        for (Counter i = 0, j = _M_engine->GetModuleCount(); i < j; i++)
        {
            _M_engine->GetModuleByIndex(i)->BindAllImportedFunctions();
        }
        return *this;
    }

    ScriptEngine& ScriptEngine::funcdef(const char* declaration)
    {
        _M_engine->RegisterFuncdef(declaration);
        return *this;
    }

    ScriptEngine& ScriptEngine::funcdef(const String& declaration)
    {
        return funcdef(declaration.c_str());
    }

    ScriptEngine& ScriptEngine::register_typedef(const char* type, const char* declaration)
    {
        _M_engine->RegisterTypedef(type, declaration);
        return *this;
    }

    ScriptEngine& ScriptEngine::register_typedef(const String& type, const String& declaration)
    {
        return register_typedef(type.c_str(), declaration.c_str());
    }

    ScriptObject ScriptEngine::create_script_object(const ScriptTypeInfo& info, bool uninited) const
    {
        if (info.is_valid())
        {
            if (uninited)
            {
                return reinterpret_cast<asIScriptObject*>(_M_engine->CreateUninitializedScriptObject(info._M_info));
            }
            else
            {
                return reinterpret_cast<asIScriptObject*>(_M_engine->CreateScriptObject(info._M_info));
            }
        }

        return {};
    }

    ScriptEngine& ScriptEngine::destroy_script_object(ScriptObjectAddress object, const ScriptTypeInfo& info)
    {
        _M_engine->ReleaseScriptObject(object, info._M_info);
        return *this;
    }

    ScriptEngine::NamespaceSaverScoped::NamespaceSaverScoped() : _M_ns(ScriptEngine::instance()->default_namespace())
    {}

    ScriptEngine::NamespaceSaverScoped::~NamespaceSaverScoped()
    {
        ScriptEngine::instance()->default_namespace(_M_ns);
    }


    uint_t ScriptEngine::global_function_count() const
    {
        return _M_engine->GetGlobalFunctionCount();
    }

    ScriptFunction ScriptEngine::global_function_by_index(uint_t index) const
    {
        return ScriptFunction(_M_engine->GetGlobalFunctionByIndex(index)).bind();
    }

    ScriptFunction ScriptEngine::global_function_by_decl(const char* declaration) const
    {
        return ScriptFunction(_M_engine->GetGlobalFunctionByDecl(declaration)).bind();
    }

    ScriptFunction ScriptEngine::global_function_by_decl(const String& declaration) const
    {
        return global_function_by_decl(declaration.c_str());
    }

    ScriptEngine& ScriptEngine::garbage_collect(BitMask flags, size_t iterations)
    {
        _M_engine->GarbageCollect(flags, iterations);
        return *this;
    }

    uint_t ScriptEngine::object_type_count() const
    {
        return _M_engine->GetObjectTypeCount();
    }

    ScriptTypeInfo ScriptEngine::object_type_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_engine->GetObjectTypeByIndex(index)).bind();
    }

    // Enums
    uint_t ScriptEngine::enum_count() const
    {
        return _M_engine->GetEnumCount();
    }

    ScriptTypeInfo ScriptEngine::enum_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_engine->GetEnumByIndex(index)).bind();
    }

    // Funcdefs
    uint_t ScriptEngine::funcdef_count() const
    {
        return _M_engine->GetFuncdefCount();
    }

    ScriptTypeInfo ScriptEngine::funcdef_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_engine->GetFuncdefByIndex(index)).bind();
    }

    // Typedefs
    uint_t ScriptEngine::typedef_count() const
    {
        return _M_engine->GetTypedefCount();
    }

    ScriptTypeInfo ScriptEngine::typedef_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_engine->GetTypedefByIndex(index)).bind();
    }

    // Script modules
    ScriptEngine& ScriptEngine::discard_module(const char* module_name)
    {
        _M_engine->DiscardModule(module_name);
        return *this;
    }

    ScriptEngine& ScriptEngine::discard_module(const String& module_name)
    {
        return discard_module(module_name.c_str());
    }

    ScriptModule ScriptEngine::module_by_index(uint_t index) const
    {
        return ScriptModule(_M_engine->GetModuleByIndex(index));
    }

    // Script functions
    int_t ScriptEngine::last_function_id() const
    {
        return _M_engine->GetLastFunctionId();
    }

    ScriptFunction ScriptEngine::function_by_id(int func_id) const
    {
        return ScriptFunction(_M_engine->GetFunctionById(func_id)).bind();
    }

    // Type identification
    int_t ScriptEngine::typeid_by_decl(const char* decl) const
    {
        return _M_engine->GetTypeIdByDecl(decl);
    }

    const char* ScriptEngine::type_declaration(int type_id, bool include_namespace) const
    {
        return _M_engine->GetTypeDeclaration(type_id, include_namespace);
    }

    int_t ScriptEngine::sizeof_primitive_type(int type_id) const
    {
        return _M_engine->GetSizeOfPrimitiveType(type_id);
    }

    ScriptTypeInfo ScriptEngine::type_info_by_id(int type_id) const
    {
        return ScriptTypeInfo(_M_engine->GetTypeInfoById(type_id)).bind();
    }

    ScriptTypeInfo ScriptEngine::type_info_by_name(const char* name) const
    {
        return ScriptTypeInfo(_M_engine->GetTypeInfoByName(name)).bind();
    }

    ScriptTypeInfo ScriptEngine::type_info_by_decl(const char* decl) const
    {
        return ScriptTypeInfo(_M_engine->GetTypeInfoByDecl(decl)).bind();
    }

    ScriptTypeInfo ScriptEngine::type_info_by_name(const String& name) const
    {
        return type_info_by_name(name.c_str());
    }

    ScriptTypeInfo ScriptEngine::type_info_by_decl(const String& decl) const
    {
        return type_info_by_decl(decl.c_str());
    }


}// namespace Engine
