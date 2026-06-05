#pragma once
#include <Core/etl/registry.hpp>

namespace Trinex
{
	class ENGINE_EXPORT TickableObject : public Registry<TickableObject>
	{
		trinex_registry(TickableObject);

	public:
		virtual TickableObject& begin_frame();
		virtual TickableObject& update(float dt);
		virtual TickableObject& end_frame();
		virtual bool is_tickable() const;

		template<typename Func, typename... Args>
		static void for_each(Func&& func, Args... args)
		{
			TickableObject* head = static_first();

			while (head)
			{
				if (head->is_tickable())
				{
					func(head, args...);
				}

				head = head->next();
			}
		}

		template<typename Ret, typename... Args>
		static void for_each_invoke(Ret (TickableObject::*method)(Args...), Args... args)
		{
			TickableObject* head = static_first();

			while (head)
			{
				if (head->is_tickable())
				{
					(head->*method)(args...);
				}

				head = head->next();
			}
		}

		template<typename Ret, typename... Args>
		static void for_each_invoke(Ret (TickableObject::*method)(Args...) const, Args... args)
		{
			TickableObject* head = static_first();

			while (head)
			{
				if (head->is_tickable())
				{
					(head->*method)(args...);
				}

				head = head->next();
			}
		}
	};
}// namespace Trinex
