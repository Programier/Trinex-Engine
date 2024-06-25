#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_object.hpp>
#include <angelscript.h>


namespace Engine
{
    static asIScriptContext* m_context = nullptr;

    void ScriptContext::initialize()
    {
        m_context = ScriptEngine::engine()->RequestContext();
        m_context->AddRef();
    }

    void ScriptContext::terminate()
    {
        m_context->Release();
        ScriptEngine::engine()->ReturnContext(m_context);
        m_context = nullptr;
    }

    int_t ScriptContext::begin_execute(const ScriptFunction& function)
    {
        auto current_state = state();
        if (current_state != State::Uninitialized && current_state != State::Active)
        {
            throw EngineException("State of context must be Uninitialized or Active!");
        }

        if (current_state == State::Active)
        {
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

        return current_state == State::Active ? 1 : 0;
    }

    void ScriptContext::end_execute(int_t begin_value, bool need_execute, void (*return_callback)(void* from, void* to),
                                    void* copy_to)
    {
        if (need_execute)
        {
            if (!execute())
            {
                unprepare();

                if (begin_value == 1)
                {
                    pop_state();
                }

                throw EngineException("Failed to execute script function!");
            }
        }

        if (return_callback && copy_to)
        {
            return_callback(address_of_return_value(), copy_to);
        }

        if (begin_value == 0 && !unprepare())
        {

            throw EngineException("Failed to unprepare function!");
        }

        if (begin_value == 1 && !pop_state())
        {
            throw EngineException("Failed to pop state!");
        }
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
        void* obj = object.object();
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

    bool ScriptContext::arg(uint_t arg, void* addr)
    {
        return m_context->SetArgAddress(arg, addr) >= 0;
    }

    bool ScriptContext::arg(uint_t arg, const ScriptObject& object)
    {
        if (void* obj = object.object())
        {
            return m_context->SetArgObject(arg, obj) >= 0;
        }
        return false;
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
        return ScriptObject(reinterpret_cast<asIScriptObject*>(m_context->GetReturnObject()));
    }

    void* ScriptContext::address_of_return_value()
    {
        return m_context->GetAddressOfReturnValue();
    }

    bool ScriptContext::exception(const char* info, bool allow_catch)
    {
        return m_context->SetException(info, allow_catch) >= 0;
    }

    int_t ScriptContext::exception_line_number(int_t* column, const char** section_name)
    {
        return m_context->GetExceptionLineNumber(column, section_name);
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


    ScriptFunction ScriptContext::function(uint_t stack_level)
    {
        return ScriptFunction(m_context->GetFunction(stack_level));
    }

    ScriptFunction ScriptContext::system_function()
    {
        return ScriptFunction(m_context->GetSystemFunction());
    }
}// namespace Engine
