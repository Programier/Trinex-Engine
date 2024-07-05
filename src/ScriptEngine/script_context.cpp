#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_handle.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_primitives.hpp>
#include <angelscript.h>
#include <scriptdictionary.h>
#include <scripthelper.h>

namespace Engine
{
    static asIScriptContext* m_context      = nullptr;
    static Function<void(void*)> m_callback = {};

    struct ExecInfo {
        int_t return_type_id = 0;
        bool is_active       = false;
    };

    static List<ExecInfo> m_exec_info;


    static void script_line_callback_internal(asIScriptEngine* engine, void* userdata)
    {
        return m_callback(userdata);
    }

    void ScriptContext::initialize()
    {
        m_context = ScriptEngine::engine()->RequestContext();
        m_context->AddRef();
    }

    void ScriptContext::terminate()
    {
        clear_line_callback();
        m_context->Release();
        ScriptEngine::engine()->ReturnContext(m_context);
        m_context = nullptr;
    }

    ScriptContext& ScriptContext::instance()
    {
        static ScriptContext context;
        return context;
    }


    bool ScriptContext::begin_execute(const ScriptFunction& function)
    {
        auto current_state = state();
        if (!is_in<State::Uninitialized, State::Active, State::Finished>(current_state))
        {
            if (current_state == State::Exception)
            {
                throw EngineException(exception_string());
            }
            throw EngineException("State of context must be Uninitialized or Active!");
        }

        if (current_state == State::Finished)
        {
            return false;
        }

        ExecInfo info;
        info.return_type_id = function.return_type_id();

        if (current_state == State::Active)
        {
            info.is_active = true;
            if (!push_state())
            {
                throw EngineException("Failed to push new state!");
            }
        }

        if (!prepare(function))
        {
            if (current_state == State::Active)
            {
                pop_state();
            }
            throw EngineException("Failed to prepare function!");
        }

        m_exec_info.emplace_back(std::move(info));
        return true;
    }

    ScriptVariable ScriptContext::end_execute(bool need_execute)
    {
        if (m_exec_info.empty())
            throw EngineException("ScriptContext::end_execute: call begin_execute before calling this method!");

        ExecInfo info = m_exec_info.back();
        m_exec_info.pop_back();

        if (need_execute)
        {
            if (!execute())
            {
                unprepare();

                if (info.is_active == true)
                {
                    pop_state();
                }

                throw EngineException("Failed to execute script function!");
            }
        }

        ScriptVariable return_variable;

        if (info.return_type_id != 0)
        {
            return_variable.create(address_of_return_value(), info.return_type_id);
        }

        if (!info.is_active && !unprepare())
        {

            throw EngineException("Failed to unprepare function!");
        }

        if (info.is_active && !pop_state())
        {
            throw EngineException("Failed to pop state!");
        }

        return return_variable;
    }

    asIScriptContext* ScriptContext::context()
    {
        return m_context;
    }

    bool ScriptContext::prepare(const ScriptFunction& func)
    {
        return m_context->Prepare(func.function()) >= 0;
    }

    bool ScriptContext::unprepare()
    {
        return m_context->Unprepare() >= 0;
    }

    bool ScriptContext::execute()
    {
        return m_context->Execute() >= 0;
    }

    bool ScriptContext::abort()
    {
        return m_context->Abort() >= 0;
    }

    bool ScriptContext::suspend()
    {
        return m_context->Suspend() >= 0;
    }

    ScriptContext::State ScriptContext::state()
    {
        auto state = m_context->GetState();
        switch (state)
        {
            case asEXECUTION_FINISHED:
                return State::Finished;
            case asEXECUTION_SUSPENDED:
                return State::Suspended;
            case asEXECUTION_ABORTED:
                return State::Aborted;
            case asEXECUTION_EXCEPTION:
                return State::Exception;
            case asEXECUTION_PREPARED:
                return State::Prepared;
            case asEXECUTION_UNINITIALIZED:
                return State::Uninitialized;
            case asEXECUTION_ACTIVE:
                return State::Active;
            case asEXECUTION_ERROR:
                return State::Error;
            case asEXECUTION_DESERIALIZATION:
                return State::Deserealization;
            default:
                return State::Undefined;
        }
    }

    bool ScriptContext::push_state()
    {
        return m_context->PushState() >= 0;
    }

    bool ScriptContext::pop_state()
    {
        return m_context->PopState() >= 0;
    }

    uint_t ScriptContext::nest_count()
    {
        asUINT count = 0;
        if (m_context->IsNested(&count))
        {
            return static_cast<uint_t>(count);
        }
        return 0;
    }

    bool ScriptContext::object(const ScriptObject& object)
    {
        void* obj = object.address();
        if (obj == nullptr)
            return false;
        return m_context->SetObject(obj) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, byte value)
    {
        return m_context->SetArgByte(arg, value) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, word value)
    {
        return m_context->SetArgWord(arg, value) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, dword value)
    {
        return m_context->SetArgDWord(arg, value) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, qword value)
    {
        return m_context->SetArgQWord(arg, value) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, float value)
    {
        return m_context->SetArgFloat(arg, value) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, double value)
    {
        return m_context->SetArgDouble(arg, value) >= 0;
    }

    bool ScriptContext::arg_reference(uint_t arg, void* addr)
    {
        return m_context->SetArgAddress(arg, addr) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, void* addr)
    {
        return m_context->SetArgObject(arg, addr) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, void* ptr, int_t type_id)
    {
        return m_context->SetArgVarType(arg, ptr, type_id);
    }

    void* ScriptContext::address_of_arg(uint_t arg)
    {
        return m_context->GetAddressOfArg(arg);
    }

    uint8_t ScriptContext::return_byte()
    {
        return static_cast<uint8_t>(m_context->GetReturnByte());
    }

    uint16_t ScriptContext::return_word()
    {
        return static_cast<uint16_t>(m_context->GetReturnWord());
    }

    uint32_t ScriptContext::return_dword()
    {
        return static_cast<uint32_t>(m_context->GetReturnDWord());
    }

    uint64_t ScriptContext::return_qword()
    {
        return static_cast<uint64_t>(m_context->GetReturnQWord());
    }

    float ScriptContext::return_float()
    {
        return m_context->GetReturnFloat();
    }

    double ScriptContext::return_double()
    {
        return m_context->GetReturnDouble();
    }

    void* ScriptContext::return_address()
    {
        return m_context->GetReturnAddress();
    }

    ScriptObject ScriptContext::return_object()
    {
        int_t return_typeid = function(0).return_type_id();
        if (return_typeid & asTYPEID_MASK_OBJECT)
        {
            return ScriptObject(m_context->GetReturnObject(), ScriptEngine::type_info_by_id(return_typeid));
        }
        return {};
    }

    void* ScriptContext::address_of_return_value()
    {
        return m_context->GetAddressOfReturnValue();
    }

    bool ScriptContext::exception(const char* info, bool allow_catch)
    {
        return m_context->SetException(info, allow_catch) >= 0;
    }

    bool ScriptContext::exception(const String& info, bool allow_catch)
    {
        return exception(info.c_str(), allow_catch);
    }

    IntVector2D ScriptContext::exception_line_position(StringView* section)
    {
        IntVector2D result = {-1, -1};
        const char* name   = nullptr;
        result.y           = m_context->GetExceptionLineNumber(&result.x, section ? &name : nullptr);

        if (section && name)
        {
            (*section) = name;
        }
        return result;
    }

    ScriptFunction ScriptContext::exception_function()
    {
        return ScriptFunction(m_context->GetExceptionFunction());
    }

    String ScriptContext::exception_string()
    {
        if (const char* text = m_context->GetExceptionString())
        {
            return text;
        }
        return "";
    }

    bool ScriptContext::will_exception_be_caught()
    {
        return m_context->WillExceptionBeCaught();
    }

    bool ScriptContext::line_callback(const Function<void(void*)>& function, void* userdata)
    {
        m_callback        = function;
        const bool result = m_context->SetLineCallback(asFUNCTION(script_line_callback_internal), userdata, asCALL_CDECL) >= 0;
        if (!result)
            clear_line_callback();
        return result;
    }

    bool ScriptContext::line_callback(const ScriptFunction& function)
    {
        return line_callback(
                [function](void*) {
                    m_context->ClearLineCallback();
                    ScriptContext::execute(function);
                    m_context->SetLineCallback(asFUNCTION(script_line_callback_internal), nullptr, asCALL_CDECL);
                },
                nullptr);
    }

    ScriptContext& ScriptContext::clear_line_callback()
    {
        Function<void(void*)> tmp = {};
        m_callback.swap(tmp);
        return instance();
    }

    uint_t ScriptContext::callstack_size()
    {
        return m_context->GetCallstackSize();
    }

    ScriptFunction ScriptContext::function(uint_t stack_level)
    {
        return ScriptFunction(m_context->GetFunction(stack_level));
    }

    ScriptFunction ScriptContext::system_function()
    {
        return ScriptFunction(m_context->GetSystemFunction());
    }

    IntVector2D ScriptContext::line_position(uint_t stack_level, StringView* section_name)
    {
        IntVector2D result  = {-1, -1};
        const char* section = nullptr;
        result.y            = m_context->GetLineNumber(stack_level, &result.x, (section_name ? &section : nullptr));

        if (section_name)
        {
            (*section_name) = section ? section : "";
        }

        return result;
    }

    uint_t ScriptContext::var_count(uint_t stack_level)
    {
        const int count = m_context->GetVarCount(stack_level);
        return count > 0 ? static_cast<uint_t>(count) : 0;
    }

    bool ScriptContext::var(uint_t var_index, uint_t stack_level, StringView* name, int_t* type_id,
                            Flags<ScriptTypeModifiers>* modifiers, bool* is_var_on_heap, int_t* stack_offset)
    {
        asETypeModifiers script_modifiers;
        const char* script_name;
        const bool result = m_context->GetVar(var_index, stack_level, &script_name, type_id,
                                              (modifiers ? &script_modifiers : nullptr), is_var_on_heap, stack_offset);

        if (name)
        {
            (*name) = script_name ? script_name : "";
        }

        if (modifiers)
        {
            (*modifiers) = Flags<ScriptTypeModifiers>(static_cast<BitMask>(script_modifiers));
        }
        return result;
    }

    String ScriptContext::var_declaration(uint_t var_index, uint_t stack_level, bool include_namespace)
    {
        if (auto decl = m_context->GetVarDeclaration(var_index, stack_level, include_namespace))
            return decl;
        return "";
    }

    void* ScriptContext::address_of_var(uint_t var_index, uint_t stack_level, bool dont_dereference,
                                        bool return_address_of_unitialized_objects)
    {
        return m_context->GetAddressOfVar(var_index, stack_level, dont_dereference, return_address_of_unitialized_objects);
    }

    bool ScriptContext::is_var_in_scope(uint_t var_index, uint_t stack_level)
    {
        return m_context->IsVarInScope(var_index, stack_level);
    }

    int_t ScriptContext::this_type_id(uint_t stack_level)
    {
        return m_context->GetThisTypeId(stack_level);
    }

    void* ScriptContext::this_pointer(uint_t stack_level)
    {
        return m_context->GetThisPointer(stack_level);
    }

    static void bind_script_argument(asUINT argument, const CScriptDictionary::CIterator& it, int_t param_typeid)
    {
        int type_id = it.GetTypeId();

        if (type_id != asTYPEID_VOID)
        {
            if (type_id & asTYPEID_OBJHANDLE)
            {
                void* value = nullptr;
                it.GetValue(&value, type_id);
                ScriptContext::arg_reference(argument, value);
            }
            else if (type_id & asTYPEID_MASK_OBJECT)
            {
                ScriptContext::arg(argument, const_cast<void*>(it.GetAddressOfValue()));
            }
            else
            {
                union
                {
                    qword qword_value = 0;
                    dword dword_value;
                    word word_value;
                    byte byte_value;
                    float float_value;
                    double double_value;
                };

                it.GetValue(&qword_value, type_id);

                if (is_in<asTYPEID_BOOL, asTYPEID_INT8, asTYPEID_UINT8>(param_typeid))
                {
                    ScriptContext::arg(argument, byte_value);
                }
                else if (is_in<asTYPEID_INT16, asTYPEID_UINT16>(param_typeid))
                {
                    ScriptContext::arg(argument, word_value);
                }
                else if (is_in<asTYPEID_INT32, asTYPEID_UINT32>(param_typeid))
                {
                    ScriptContext::arg(argument, dword_value);
                }
                else if (is_in<asTYPEID_INT64, asTYPEID_UINT64>(param_typeid))
                {
                    ScriptContext::arg(argument, qword_value);
                }
                else if (type_id == asTYPEID_FLOAT)
                {
                    ScriptContext::arg(argument, float_value);
                }
                else if (type_id == asTYPEID_DOUBLE)
                {
                    ScriptContext::arg(argument, double_value);
                }
            }
        }
    }

    static bool script_execute_function(const ScriptFunction& function, const CScriptDictionary& args)
    {
        ScriptContext::begin_execute(function);
        uint_t count = function.param_count();

        for (uint_t i = 0; i < count; ++i)
        {
            StringView name;
            int_t type_id = 0;
            StringView default_param;
            function.param(i, &type_id, nullptr, &name, &default_param);

            auto it = args.find(String(name));

            if (it == args.end())
            {
                // Maybe we can get object from default param string?
                // We can compile the function at runtime, but this will significantly affect performance
                //if (default_param.empty())
                {
                    error_log("ScriptContext", "Cannot execute function '%s', because parameter '%s' is not found!",
                              function.name().data(), name.data());
                    ScriptContext::end_execute(false);
                    return false;
                }

                continue;
            }

            bind_script_argument(i, it, type_id);
        }

        ScriptContext::end_execute(true);
        return true;
    }

    static bool script_var(uint_t var_index, uint_t stack_level, ScriptPointer* name, Integer32* type_id, Integer32* modifiers,
                           Boolean* is_var_on_heap, Integer32* stack_offset)
    {
        StringView* out_name = name ? name->as<StringView>() : nullptr;
        int_t* out_type_id   = type_id ? &type_id->value : nullptr;
        Flags<ScriptTypeModifiers> out_modifiers;
        bool* out_is_var_on_heap = is_var_on_heap ? &is_var_on_heap->value : nullptr;
        int_t* out_stack_offset  = stack_offset ? &stack_offset->value : nullptr;

        const bool result = ScriptContext::var(var_index, stack_level, out_name, out_type_id,
                                               modifiers ? &out_modifiers : nullptr, out_is_var_on_heap, out_stack_offset);

        if (modifiers)
        {
            modifiers->value = static_cast<int_t>(out_modifiers);
        }

        return result;
    }

    static void on_init()
    {
        ScriptEngine::NamespaceSaverScoped saver;

        ScriptEngine::default_namespace("Engine::ScriptContext");
        ScriptEngine::register_function("bool execute(const ScriptFunction& in, const dictionary& in = {})",
                                        script_execute_function);

        ScriptEngine::register_function("bool exception(const string& in, bool = true)",
                                        func_of<bool(const String&, bool)>(ScriptContext::exception));
        ScriptEngine::register_function("IntVector2D exception_line_position(Ptr<StringView>@ = null)",
                                        func_of<IntVector2D(ScriptPointer*)>([](ScriptPointer* pointer) -> IntVector2D {
                                            return ScriptContext::exception_line_position(pointer ? pointer->as<StringView>()
                                                                                                  : nullptr);
                                        }));
        ScriptEngine::register_function("ScriptFunction exception_function()", ScriptContext::exception_function);
        ScriptEngine::register_function("string exception_string()", ScriptContext::exception_string);
        ScriptEngine::register_function("bool will_exception_be_caught()", ScriptContext::will_exception_be_caught);

        ScriptEngine::register_function("bool line_callback(const Engine::ScriptFunction& in)",
                                        func_of<bool(const ScriptFunction&)>(&ScriptContext::line_callback));
        ScriptEngine::register_function("void clear_line_callback()", &ScriptContext::clear_line_callback);
        ScriptEngine::register_function("uint callstack_size()", &ScriptContext::callstack_size);
        ScriptEngine::register_function("ScriptFunction function(uint = 0)", &ScriptContext::function);
        ScriptEngine::register_function(
                "IntVector2D line_position(uint level = 0, Ptr<StringView>@ = null)",
                func_of<IntVector2D(uint_t, ScriptPointer*)>([](uint_t stack_level, ScriptPointer* pointer) -> IntVector2D {
                    return ScriptContext::line_position(stack_level, pointer ? pointer->as<StringView>() : nullptr);
                }));

        ScriptEngine::register_function("uint var_count(uint = 0)", ScriptContext::var_count);

        ScriptEngine::register_function(
                "string var_declaration(uint var_index, uint stack_level = 0, bool include_namespace = false)",
                ScriptContext::var_declaration);
        ScriptEngine::register_function("bool is_var_in_scope(uint var_index, uint = 0)", ScriptContext::is_var_in_scope);
        ScriptEngine::register_function("int this_type_id(uint = 0)", ScriptContext::this_type_id);
        ScriptEngine::register_function("ScriptFunction system_function()", ScriptContext::system_function);
        ScriptEngine::register_function(
                "bool var(uint, uint, Ptr<StringView>@ = nullptr, Int32@ = null, Int32@ = null, Bool@ = null, Int32@ = null)",
                script_var);
    }

    static ReflectionInitializeController initilizer(on_init, "Engine::ScriptContext",
                                                     {"Engine::ScriptFunction", "Engine::IntVector", "Engine::ScriptEnums"});
}// namespace Engine
