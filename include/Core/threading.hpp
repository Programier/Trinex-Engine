#pragma once
#include <Core/task.hpp>
#include <Core/thread.hpp>

namespace Engine
{
	ENGINE_EXPORT void create_threads();
	ENGINE_EXPORT void destroy_threads();

	ENGINE_EXPORT Thread* render_thread();
	ENGINE_EXPORT Thread* logic_thread();
	ENGINE_EXPORT Thread* this_thread();

	ENGINE_EXPORT bool is_in_render_thread();
	ENGINE_EXPORT bool is_in_logic_thread();


#define is_in_logic_thread_checked()                                                                                             \
	if (!is_in_logic_thread())                                                                                                   \
		throw EngineException("Not in logic thread");

#define is_in_render_thread_checked()                                                                                            \
	if (!is_in_render_thread())                                                                                                  \
		throw EngineException("Not in render thread");

	template<typename Variable>
	class UpdateVariableCommand : public Task<UpdateVariableCommand<Variable>>
	{
		Variable src_variable;
		Variable& dst_variable;

	public:
		UpdateVariableCommand(const Variable& src, Variable& dst) : src_variable(src), dst_variable(dst) {}

		void execute() override { dst_variable = src_variable; }
	};

	template<typename Callable>
	FORCE_INLINE void call_in_render_thread(Callable&& callable)
	{
		struct Command : public Task<Command> {
			Callable m_callable;

			Command(Callable&& callable) : m_callable(std::forward<Callable>(callable)) {}

			void execute() override { m_callable(); }
		};

		Thread* rt = render_thread();
		if (ThisThread::self() == rt)
		{
			callable();
		}
		else
		{
			rt->create_task<Command>(std::forward<Callable>(callable));
		}
	}

	template<typename Callable>
	FORCE_INLINE void call_in_logic_thread(Callable&& callable)
	{
		struct Command : public Task<Command> {
			Callable m_callable;

			Command(Callable&& callable) : m_callable(std::forward<Callable>(callable)) {}

			void execute() override { m_callable(); }
		};

		Thread* rt = logic_thread();
		if (ThisThread::self() == rt)
		{
			callable();
		}
		else
		{
			rt->create_task<Command>(std::forward<Callable>(callable));
		}
	}

}// namespace Engine
