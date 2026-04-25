#include <Core/etl/flat_map.hpp>
#include <Core/lifecycle.hpp>
#include <cstring>

namespace Trinex::LifeCycle
{
	namespace
	{
		struct Handler {
			Handler* next;
			Function function;
			const char* name;
			Vector<const char*> dependencies;
		};

		struct PendingHandlers {
			PendingHandlers* next;
			Handler* list;
		};

		static void execute_dependencies(const char* dependency, Handler** list);

		static FlatMap<i32, Handler*>& handlers()
		{
			static FlatMap<i32, Handler*> s_handlers;
			return s_handlers;
		}

		static Handler* s_stack = nullptr;

		static bool name_equal(const char* a, const char* b)
		{
			if (!a || !b)
				return false;

			return strcmp(a, b) == 0;
		}

		static void execute_handler(Handler* current, Handler** pending)
		{
			trinex_assert(current->next == nullptr);
			current->next = s_stack;
			s_stack       = current;

			while (!current->dependencies.empty())
			{
				const char* dependency = current->dependencies.back();
				current->dependencies.pop_back();

				execute_dependencies(dependency, pending);
			}

			if (current->function)
			{
				current->function();
			}

			trinex_assert(s_stack == current);
			s_stack = current->next;
			trx_delete current;
		}

		static void execute_dependencies(const char* dependency, Handler** list)
		{
			if (list == nullptr)
				return;

			while (*list)
			{
				if (name_equal(dependency, (*list)->name))
				{
					Handler* next = (*list);
					(*list)       = (*list)->next;
					next->next    = nullptr;

					execute_handler(next, list);
				}
				else
				{
					list = &(*list)->next;
				}
			}
		}
	}// namespace

	Registrar::Registrar(i32 event, Function function, const Description& description)
	{
		Handler*& list = handlers()[event];
		list           = trx_new Handler{list, function, description.name, description.after};
	}

	ENGINE_EXPORT void execute(i32 event)
	{
		Handler*& list = handlers()[event];

		while (list)
		{
			Handler* current = list;
			list             = list->next;
			current->next    = nullptr;

			execute_handler(current, &list);
		}
	}
}// namespace Trinex::LifeCycle
