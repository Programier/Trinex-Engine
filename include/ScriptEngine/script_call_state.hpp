#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_variable.hpp>

namespace Engine
{
	class ENGINE_EXPORT ScriptCallState final
	{
		ScriptFunction m_function;
		void* m_object = 0;

		dword m_stack_frame_pointer = 0;
		dword m_program_pointer     = 0;
		dword m_stack_pointer       = 0;
		dword m_stack_index         = 0;

		Map<String, ScriptVariable> m_local_variables;

	public:
		ScriptCallState& clear();
		ScriptCallState& save(uint_t stack_level, bool save_local_variables = true, bool save_arguments = true);
		const ScriptCallState& restore() const;
	};
}// namespace Engine
