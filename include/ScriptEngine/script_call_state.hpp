#include <Core/etl/map.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_variable.hpp>

namespace Trinex
{
	class ENGINE_EXPORT ScriptCallState final
	{
		ScriptFunction m_function;
		void* m_object = 0;

		u32 m_stack_frame_pointer = 0;
		u32 m_program_pointer     = 0;
		u32 m_stack_pointer       = 0;
		u32 m_stack_index         = 0;

		Map<String, ScriptVariable> m_local_variables;

	public:
		ScriptCallState& clear();
		ScriptCallState& save(u32 stack_level, bool save_local_variables = true, bool save_arguments = true);
		const ScriptCallState& restore() const;
	};
}// namespace Trinex
