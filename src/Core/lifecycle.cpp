module Trinex.Core.LifeCycle;
import Trinex.Core.Assert;

namespace Trinex::LifeCycle
{
	namespace
	{
		struct Handler
		{
			Handler* next;
			Function function;
			StringView name;
			Array<StringView> dependencies;
		};

		struct PendingHandlers
		{
			PendingHandlers* next;
			Handler* list;
		};

		static void execute_handler(Handler** list, StringView dependency);

		static FlatMap<i32, Handler*>& handlers()
		{
			static FlatMap<i32, Handler*> s_handlers;
			return s_handlers;
		}

		static Handler* s_stack = nullptr;

		static void execute_handler(Handler* current, Handler** pending)
		{
			Assert::assertion(current->next == nullptr, "Lifecycle handler is already executing");
			current->next = s_stack;
			s_stack       = current;

			while (!current->dependencies.empty())
			{
				StringView dependency = current->dependencies.back();
				current->dependencies.pop_back();

				execute_handler(pending, dependency);
			}

			if (current->function)
			{
				current->function();
			}

			Assert::assertion(s_stack == current, "Lifecycle handler stack is corrupted");
			s_stack = current->next;
			delete current;
		}

		static void execute_handler(Handler** list, StringView dependency)
		{
			if (list == nullptr)
				return;

			while (*list)
			{
				if (dependency == (*list)->name)
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
	} // namespace

	Registrar::Registrar(i32 event, Function function, const Description& description)
	{
		Handler*& list = handlers()[event];
		list           = new Handler{list, function, description.name, description.deps};
	}

	trinex_api void execute(i32 stage)
	{
		Handler*& list = handlers()[stage];

		while (list)
		{
			Handler* current = list;
			list             = list->next;
			current->next    = nullptr;

			execute_handler(current, &list);
		}
	}

	trinex_api void execute(i32 stage, StringView handler)
	{
		execute_handler(&handlers()[stage], handler);
	}
} // namespace Trinex::LifeCycle
