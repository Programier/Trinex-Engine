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
		trinex_non_copyable(Registry);
		trinex_non_moveable(Registry);

		static inline RegistryInstance* static_first() { return RegistryInstance::s_first; }
		static inline RegistryInstance* static_last() { return RegistryInstance::s_last; }

		template<typename Func, typename... Args>
		static void for_each(Func&& func, Args... args)
		{
			RegistryInstance* head = static_first();

			while (head)
			{
				func(head, args...);
				head = head->next();
			}
		}

		template<typename Ret, typename... Args>
		static void for_each_invoke(Ret (RegistryInstance::*method)(Args...), Args... args)
		{
			RegistryInstance* head = static_first();

			while (head)
			{
				(head->*method)(args...);
				head = head->next();
			}
		}

		template<typename Ret, typename... Args>
		static void for_each_invoke(Ret (RegistryInstance::*method)(Args...) const, Args... args)
		{
			RegistryInstance* head = static_first();

			while (head)
			{
				(head->*method)(args...);
				head = head->next();
			}
		}

		inline RegistryInstance* prev() const { return m_prev; }
		inline RegistryInstance* next() const { return m_next; }

		virtual ~Registry()
		{
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
	static inline Type* static_first()                                                                                           \
	{                                                                                                                            \
		return s_first;                                                                                                          \
	}                                                                                                                            \
	static inline Type* static_last()                                                                                            \
	{                                                                                                                            \
		return s_last;                                                                                                           \
	}                                                                                                                            \
	friend class Registry;                                                                                                       \
                                                                                                                                 \
private:                                                                                                                         \
	static Type* s_first;                                                                                                        \
	static Type* s_last


#define trinex_implement_registry(Type)                                                                                          \
	Type* Type::s_first = nullptr;                                                                                               \
	Type* Type::s_last  = nullptr
}// namespace Trinex
