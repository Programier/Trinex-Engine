#include <Core/string_functions.hpp>
#include <ScriptEngine/script_call_state.hpp>
#include <ScriptEngine/script_context.hpp>
#include <angelscript.h>

namespace Engine
{
	ScriptCallState& ScriptCallState::clear()
	{
		m_local_variables.clear();
		return *this;
	}

	ScriptCallState& ScriptCallState::save(uint_t stack_level, bool save_local_variables, bool save_arguments)
	{
		asIScriptContext* context   = ScriptContext::context();
		asIScriptFunction* function = nullptr;
		context->GetCallStateRegisters(stack_level, reinterpret_cast<asDWORD*>(&m_stack_frame_pointer), &function,
		                               reinterpret_cast<asDWORD*>(&m_program_pointer),
		                               reinterpret_cast<asDWORD*>(&m_stack_pointer), reinterpret_cast<asDWORD*>(&m_stack_index));
		m_function = function;
		m_object   = context->GetThisPointer();

		if (save_local_variables)
		{
			// Serialize variables
			const uint_t var_count = ScriptContext::var_count(stack_level);
			for (uint_t var = 0; var < var_count; ++var)
			{
				StringView name;
				int_t type_id;
				int_t offset = 0;
				ScriptContext::var(var, stack_level, &name, &type_id, nullptr, nullptr, &offset);
				void* address = ScriptContext::address_of_var(var, stack_level);

				ScriptVariable variable(address, type_id);

				String final_name             = name.empty() ? Strings::format("{}", offset) : String(name);
				m_local_variables[final_name] = variable;
			}
		}

		return *this;
	}

	const ScriptCallState& ScriptCallState::restore() const
	{
		asIScriptContext* context = ScriptContext::context();
		context->StartDeserialization();
		context->PushFunction(m_function.function(), m_object);
		context->SetCallStateRegisters(0, m_stack_frame_pointer, m_function.function(), m_program_pointer, m_stack_pointer,
		                               m_stack_index);

		// Deserialize variables
		const uint_t var_count = ScriptContext::var_count(0);

		for (uint_t var = 0; var < var_count; ++var)
		{
			StringView name;
			int_t type_id;
			int_t offset;
			ScriptContext::var(var, 0, &name, &type_id, nullptr, nullptr, &offset);
			void* address = ScriptContext::address_of_var(var, 0, false);

			String final_name = name.empty() ? Strings::format("{}", offset) : String(name);
			auto it           = m_local_variables.find(final_name);
			if (it == m_local_variables.end())
				continue;

			const ScriptVariable& src = it->second;
			if (src.type_id() != type_id)
				continue;

			src.assign_to(address);
		}

		context->FinishDeserialization();
		return *this;
	}
}// namespace Engine
