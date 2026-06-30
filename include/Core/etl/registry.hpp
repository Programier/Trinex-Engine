#pragma once

namespace Trinex
{
	template<typename RegistryInstance>
	class Registry
	{
	private:
		RegistryInstance* m_prev = nullptr;
		RegistryInstance* m_next = nullptr;

	protected:
		Registry()
		{
			[[maybe_unused]] auto lock = RegistryInstance::critical_section();

			RegistryInstance* self = static_cast<RegistryInstance*>(this);

			if (RegistryInstance::s_last)
			{
				RegistryInstance::s_last->m_next = self;
				m_prev                           = RegistryInstance::s_last;
				RegistryInstance::s_last         = self;
			}
			else
			{
				RegistryInstance::s_first = RegistryInstance::s_last = self;
			}
		}

	public:
		struct DummyCriticalSection {
		};

		trinex_non_copyable(Registry);
		trinex_non_moveable(Registry);

		static inline RegistryInstance* first() { return RegistryInstance::s_first; }
		static inline RegistryInstance* last() { return RegistryInstance::s_last; }
		static inline DummyCriticalSection critical_section() { return {}; }

		template<typename Func, typename... Args>
		static void for_each(Func&& func, Args... args)
		{
			[[maybe_unused]] auto lock = RegistryInstance::critical_section();

			RegistryInstance* head = Registry::first();

			while (head)
			{
				func(head, args...);
				head = head->next();
			}
		}

		template<typename Ret, typename... Args>
		static void for_each_invoke(Ret (RegistryInstance::*method)(Args...), Args... args)
		{
			[[maybe_unused]] auto lock = RegistryInstance::critical_section();

			RegistryInstance* head = Registry::first();

			while (head)
			{
				(head->*method)(args...);
				head = head->next();
			}
		}

		template<typename Ret, typename... Args>
		static void for_each_invoke(Ret (RegistryInstance::*method)(Args...) const, Args... args)
		{
			[[maybe_unused]] auto lock = RegistryInstance::critical_section();

			RegistryInstance* head = Registry::first();

			while (head)
			{
				(head->*method)(args...);
				head = head->next();
			}
		}

		inline RegistryInstance* prev() const { return m_prev; }
		inline RegistryInstance* next() const { return m_next; }

		template<typename Compare>
		static void sort(Compare&& compare)
		{
			[[maybe_unused]] auto lock = RegistryInstance::critical_section();

			RegistryInstance* head = Registry::first();
			if (head == nullptr || head->m_next == nullptr)
			{
				return;
			}

			RegistryInstance* sorted = nullptr;
			RegistryInstance* tail   = nullptr;

			while (head)
			{
				RegistryInstance* node = head;
				head                   = head->m_next;
				node->m_prev           = nullptr;
				node->m_next           = nullptr;

				if (sorted == nullptr)
				{
					sorted = tail = node;
					continue;
				}

				RegistryInstance* current = sorted;
				while (current && !compare(node, current))
				{
					current = current->m_next;
				}

				if (current == sorted)
				{
					node->m_next   = sorted;
					sorted->m_prev = node;
					sorted         = node;
					continue;
				}

				if (current == nullptr)
				{
					tail->m_next = node;
					node->m_prev = tail;
					tail         = node;
					continue;
				}

				RegistryInstance* prev = current->m_prev;
				prev->m_next           = node;
				node->m_prev           = prev;
				node->m_next           = current;
				current->m_prev        = node;
			}

			RegistryInstance::s_first = sorted;
			RegistryInstance::s_last  = tail;
		}


		virtual ~Registry()
		{
			[[maybe_unused]] auto lock = RegistryInstance::critical_section();

			if (m_prev)
			{
				m_prev->m_next = m_next;
			}
			else
			{
				RegistryInstance::s_first = m_next;
			}

			if (m_next)
			{
				m_next->m_prev = m_prev;
			}
			else
			{
				RegistryInstance::s_last = m_prev;
			}
		}
	};

#define trinex_registry(Type)                                                                                                    \
public:                                                                                                                          \
	friend class Registry;                                                                                                       \
                                                                                                                                 \
private:                                                                                                                         \
	static Type* s_first;                                                                                                        \
	static Type* s_last


#define trinex_implement_registry(Type)                                                                                          \
	Type* Type::s_first = nullptr;                                                                                               \
	Type* Type::s_last  = nullptr
}// namespace Trinex
