#pragma once
#include <Core/executable_object.hpp>
#include <Core/thread.hpp>

namespace Engine
{
	ENGINE_EXPORT void create_threads();
	ENGINE_EXPORT void destroy_threads();

	ENGINE_EXPORT Thread* render_thread();
	ENGINE_EXPORT Thread* logic_thread();
	ENGINE_EXPORT ThreadBase* this_thread();

	ENGINE_EXPORT bool is_in_render_thread();
	ENGINE_EXPORT bool is_in_logic_thread();


#define is_in_logic_thread_checked()                                                                                   \
	if (!is_in_logic_thread())                                                                                         \
		throw EngineException("Not in logic thread");

#define is_in_render_thread_checked()                                                                                  \
	if (!is_in_render_thread())                                                                                        \
		throw EngineException("Not in render thread");

	template<typename Variable>
	class UpdateVariableCommand : public ExecutableObject
	{
		Variable src_variable;
		Variable& dst_variable;

	public:
		UpdateVariableCommand(const Variable& src, Variable& dst) : src_variable(src), dst_variable(dst)
		{}

		int_t execute()
		{
			dst_variable = src_variable;
			return sizeof(UpdateVariableCommand<Variable>);
		}
	};

	template<typename Callable>
	FORCE_INLINE void call_in_render_thread(Callable&& callable)
	{
		struct Command : public ExecutableObject {
			Callable m_callable;

			Command(Callable&& callable) : m_callable(std::forward<Callable>(callable))
			{}

			int_t execute() override
			{
				m_callable();
				return sizeof(Command);
			}
		};

		Thread* rt = render_thread();
		if (Thread::this_thread() == rt)
		{
			callable();
		}
		else
		{
			rt->insert_new_task<Command>(std::forward<Callable>(callable));
		}
	}

	template<typename Callable>
	FORCE_INLINE void call_in_logic_thread(Callable&& callable)
	{
		struct Command : public ExecutableObject {
			Callable m_callable;

			Command(Callable&& callable) : m_callable(std::forward<Callable>(callable))
			{}

			int_t execute() override
			{
				m_callable();
				return sizeof(Command);
			}
		};

		Thread* rt = logic_thread();
		if (Thread::this_thread() == rt)
		{
			callable();
		}
		else
		{
			rt->insert_new_task<Command>(std::forward<Callable>(callable));
		}
	}

}// namespace Engine
