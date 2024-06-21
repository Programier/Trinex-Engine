#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/stacktrace.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
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
            throw EngineException(Strings::format("{} ({}, {}): {}", msg->section, msg->row, msg->col, msg->message));
        }
    }


    ScriptEngine* ScriptEngine::m_instance = nullptr;

    ScriptEngine::ScriptEngine()
    {
        m_instance = this;

        logger->log("ScriptEngine", "Created script engine [%p]", this);
        m_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

        m_engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
        m_engine->SetEngineProperty(asEP_ALLOW_UNICODE_IDENTIFIERS, 1);
        m_engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
        m_engine->SetMessageCallback(asFUNCTION(angel_script_callback), 0, asCALL_CDECL);

#if ARCH_X86_64 || ARCH_ARM
        if constexpr (enable_jit)
        {
            info_log("ScriptEngine", "Enable JIT compiler!");
            auto compiler  = new PlatformJitCompiler();
            m_jit_compiler = compiler;
            m_engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, true);
            m_engine->SetJITCompiler(m_jit_compiler);

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

        asInitializeAddons(m_engine);

        Print::asRegister(m_engine);
        Initializers::init_primitive_wrappers();
    }

    ScriptEngine::~ScriptEngine()
    {
        m_engine->Release();
        release_scripts();
        if (m_jit_compiler)
        {
            delete m_jit_compiler;
        }
    }


    void ScriptEngine::initialize()
    {
        if (m_instance == nullptr)
        {
            new ScriptEngine();
            PostDestroyController controller(ScriptEngine::terminate);
        }
    }

    void ScriptEngine::terminate()
    {
        if (m_instance)
        {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    ScriptEngine* ScriptEngine::instance()
    {
        return m_instance;
    }

    asIScriptEngine* ScriptEngine::as_engine() const
    {
        return m_engine;
    }

    asIScriptContext* ScriptEngine::new_context() const
    {
        return m_engine->RequestContext();
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
        m_engine->RegisterGlobalFunction(declaration, asFunctionPtr(func), create_call_conv(conv));
        return *this;
    }

    const ScriptEngine& ScriptEngine::release_context(asIScriptContext* context) const
    {
        m_engine->ReturnContext(context);
        return *this;
    }

    ScriptEngine& ScriptEngine::default_namespace(const String& name)
    {
        return default_namespace(name.c_str());
    }

    ScriptEngine& ScriptEngine::default_namespace(const char* ns)
    {
        m_engine->SetDefaultNamespace(ns);
        return *this;
    }

    String ScriptEngine::default_namespace() const
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

    ScriptModule ScriptEngine::global_module() const
    {
        return m_engine->GetModule("__TRINEX_GLOBAL_MODULE__", asGM_CREATE_IF_NOT_EXISTS);
    }

    ScriptModule ScriptEngine::create_module(const String& name, EnumerateType flags) const
    {
        return create_module(name.c_str(), flags);
    }

    ScriptModule ScriptEngine::create_module(const char* name, EnumerateType flags) const
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

    ScriptModule ScriptEngine::module(uint_t index)
    {
        return m_engine->GetModuleByIndex(index);
    }

    uint_t ScriptEngine::module_count() const
    {
        return m_engine->GetModuleCount();
    }

    ScriptEngine& ScriptEngine::bind_imports()
    {
        for (Counter i = 0, j = m_engine->GetModuleCount(); i < j; i++)
        {
            m_engine->GetModuleByIndex(i)->BindAllImportedFunctions();
        }
        return *this;
    }

    ScriptEngine& ScriptEngine::funcdef(const char* declaration)
    {
        m_engine->RegisterFuncdef(declaration);
        return *this;
    }

    ScriptEngine& ScriptEngine::funcdef(const String& declaration)
    {
        return funcdef(declaration.c_str());
    }

    ScriptEngine& ScriptEngine::register_typedef(const char* type, const char* declaration)
    {
        m_engine->RegisterTypedef(type, declaration);
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
                return reinterpret_cast<asIScriptObject*>(m_engine->CreateUninitializedScriptObject(info.m_info));
            }
            else
            {
                return reinterpret_cast<asIScriptObject*>(m_engine->CreateScriptObject(info.m_info));
            }
        }

        return {};
    }

    ScriptEngine& ScriptEngine::destroy_script_object(ScriptObjectAddress object, const ScriptTypeInfo& info)
    {
        m_engine->ReleaseScriptObject(object, info.m_info);
        return *this;
    }

    ScriptEngine::NamespaceSaverScoped::NamespaceSaverScoped() : m_ns(ScriptEngine::instance()->default_namespace())
    {}

    ScriptEngine::NamespaceSaverScoped::~NamespaceSaverScoped()
    {
        ScriptEngine::instance()->default_namespace(m_ns);
    }


    uint_t ScriptEngine::global_function_count() const
    {
        return m_engine->GetGlobalFunctionCount();
    }

    ScriptFunction ScriptEngine::global_function_by_index(uint_t index) const
    {
        return ScriptFunction(m_engine->GetGlobalFunctionByIndex(index)).bind();
    }

    ScriptFunction ScriptEngine::global_function_by_decl(const char* declaration) const
    {
        return ScriptFunction(m_engine->GetGlobalFunctionByDecl(declaration)).bind();
    }

    ScriptFunction ScriptEngine::global_function_by_decl(const String& declaration) const
    {
        return global_function_by_decl(declaration.c_str());
    }

    ScriptEngine& ScriptEngine::garbage_collect(BitMask flags, size_t iterations)
    {
        m_engine->GarbageCollect(flags, iterations);
        return *this;
    }

    uint_t ScriptEngine::object_type_count() const
    {
        return m_engine->GetObjectTypeCount();
    }

    ScriptTypeInfo ScriptEngine::object_type_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_engine->GetObjectTypeByIndex(index)).bind();
    }

    bool ScriptEngine::exec_string(const String& line) const
    {
        return exec_string(line.c_str());
    }

    bool ScriptEngine::exec_string(const char* line) const
    {
        return ExecuteString(m_engine, line) >= 0;
    }

    // Enums
    uint_t ScriptEngine::enum_count() const
    {
        return m_engine->GetEnumCount();
    }

    ScriptTypeInfo ScriptEngine::enum_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_engine->GetEnumByIndex(index)).bind();
    }

    // Funcdefs
    uint_t ScriptEngine::funcdef_count() const
    {
        return m_engine->GetFuncdefCount();
    }

    ScriptTypeInfo ScriptEngine::funcdef_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_engine->GetFuncdefByIndex(index)).bind();
    }

    // Typedefs
    uint_t ScriptEngine::typedef_count() const
    {
        return m_engine->GetTypedefCount();
    }

    ScriptTypeInfo ScriptEngine::typedef_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_engine->GetTypedefByIndex(index)).bind();
    }

    // Script modules
    ScriptEngine& ScriptEngine::discard_module(const char* module_name)
    {
        m_engine->DiscardModule(module_name);
        return *this;
    }

    ScriptEngine& ScriptEngine::discard_module(const String& module_name)
    {
        return discard_module(module_name.c_str());
    }

    ScriptModule ScriptEngine::module_by_index(uint_t index) const
    {
        return ScriptModule(m_engine->GetModuleByIndex(index));
    }

    // Script functions
    int_t ScriptEngine::last_function_id() const
    {
        return m_engine->GetLastFunctionId();
    }

    ScriptFunction ScriptEngine::function_by_id(int func_id) const
    {
        return ScriptFunction(m_engine->GetFunctionById(func_id)).bind();
    }

    // Type identification
    int_t ScriptEngine::typeid_by_decl(const char* decl) const
    {
        return m_engine->GetTypeIdByDecl(decl);
    }

    const char* ScriptEngine::type_declaration(int type_id, bool include_namespace) const
    {
        return m_engine->GetTypeDeclaration(type_id, include_namespace);
    }

    int_t ScriptEngine::sizeof_primitive_type(int type_id) const
    {
        return m_engine->GetSizeOfPrimitiveType(type_id);
    }

    ScriptTypeInfo ScriptEngine::type_info_by_id(int type_id) const
    {
        return ScriptTypeInfo(m_engine->GetTypeInfoById(type_id)).bind();
    }

    ScriptTypeInfo ScriptEngine::type_info_by_name(const char* name) const
    {
        return ScriptTypeInfo(m_engine->GetTypeInfoByName(name)).bind();
    }

    ScriptTypeInfo ScriptEngine::type_info_by_decl(const char* decl) const
    {
        return ScriptTypeInfo(m_engine->GetTypeInfoByDecl(decl)).bind();
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
