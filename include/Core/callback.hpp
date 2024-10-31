#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	template<typename Signature>
	using CallBack = Function<Signature>;

	template<typename Signature>
	class CallBacks final
	{
	private:
		Vector<const CallBack<Signature>*> m_callbacks;

	public:
		Identifier push(const Function<Signature>& callback)
		{
			Function<Signature>* func = new Function<Signature>(callback);
			m_callbacks.emplace_back(func);
			return reinterpret_cast<Identifier>(func);
		}

		Identifier push(Function<Signature>&& callback)
		{
			Function<Signature>* func = new Function<Signature>(std::move(callback));
			m_callbacks.emplace_back(func);
			return reinterpret_cast<Identifier>(func);
		}

		CallBacks& remove(Identifier ID)
		{
			Function<Signature>* remove = reinterpret_cast<Function<Signature>*>(ID);

			for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
			{
				if ((*it) == remove)
				{
					delete remove;
					m_callbacks.erase(it);
					return *this;
				}
			}

			return *this;
		}

		template<typename... Args>
		const CallBacks& trigger(Args&&... args) const
		{
			for (auto* ell : m_callbacks)
			{
				(*ell)(std::forward<Args>(args)...);
			}
			return *this;
		}

		bool empty() const
		{
			return m_callbacks.empty();
		}

		template<typename... Args>
		const CallBacks& operator()(Args&&... args) const
		{
			return trigger(std::forward<Args>(args)...);
		}

		Identifier operator+=(const Function<Signature>& func)
		{
			return push(func);
		}

		Identifier operator+=(Function<Signature>&& func)
		{
			return push(func);
		}

		const Vector<const CallBack<Signature>*>& callbacks() const
		{
			return m_callbacks;
		}

		~CallBacks()
		{
			for (auto* func : m_callbacks)
			{
				delete func;
			}
			m_callbacks.clear();
		}
	};
}// namespace Engine
