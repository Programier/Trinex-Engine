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
            throw EngineException(Strings::format("{} ({}, {}): {}", msg->section, msg->row, msg->col, msg->message));
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
        _M_engine = asCreateScriptEngine();

        _M_engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);

        _M_engine->SetMessageCallback(asFUNCTION(angel_script_callback), 0, asCALL_CDECL);
        asInitializeAddons(_M_engine);
        _M_context_manager = new ScriptContextManager();
    }

    ScriptEngine::~ScriptEngine()
    {
        delete _M_context_manager;
        _M_engine->Release();
    }

    void ScriptEngine::terminate()
    {
        delete ScriptEngine::_M_instance;
        ScriptEngine::_M_instance = nullptr;
    }

    ScriptEngine* ScriptEngine::instance()
    {
        if (_M_instance == nullptr)
        {
            _M_instance = new ScriptEngine();
            PostDestroyController controller(ScriptEngine::terminate);
        }

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

    static asEGMFlags create_module_flags(ModuleCreateFlags flags)
    {
        switch (flags)
        {
            case ModuleCreateFlags::AlwaysCreate:
                return asEGMFlags::asGM_ALWAYS_CREATE;
            case ModuleCreateFlags::CreateIsNotExist:
                return asEGMFlags::asGM_CREATE_IF_NOT_EXISTS;
            case ModuleCreateFlags::OnlyIfExist:
                return asEGMFlags::asGM_ONLY_IF_EXISTS;
            default:
                return asEGMFlags::asGM_CREATE_IF_NOT_EXISTS;
        }
    }

    ScriptModule ScriptEngine::module(const char* module_name, ModuleCreateFlags flags)
    {
        return _M_engine->GetModule(module_name, create_module_flags(flags));
    }

    ScriptModule ScriptEngine::module(const String& module_name, ModuleCreateFlags flags)
    {
        return module(module_name.c_str(), flags);
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

    ScriptObject ScriptEngine::create_script_object(const ScriptTypeInfo& info) const
    {
        return reinterpret_cast<asIScriptObject*>(_M_engine->CreateScriptObject(info._M_info));
    }

    ScriptObject ScriptEngine::create_script_object_uninited(const ScriptTypeInfo& info) const
    {
        return reinterpret_cast<asIScriptObject*>(_M_engine->CreateUninitializedScriptObject(info._M_info));
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

}// namespace Engine