#pragma once
#include <Core/definitions.hpp>
#include <Core/export.hpp>

namespace Engine
{
	class ENGINE_EXPORT TickableObject
	{
	private:
		static TickableObject* s_first;
		static TickableObject* s_last;

		TickableObject* m_prev = nullptr;
		TickableObject* m_next = nullptr;

	public:
		TickableObject();
		trinex_non_copyable(TickableObject);
		trinex_non_moveable(TickableObject);

		static inline TickableObject* static_first() { return s_first; }
		static inline TickableObject* static_last() { return s_last; }

		inline TickableObject* prev() const { return m_prev; }
		inline TickableObject* next() const { return m_next; }

		virtual TickableObject& update(float dt);
		virtual bool is_tickable() const;
		virtual bool is_tickable_when_paused() const;
		virtual ~TickableObject();
	};
}// namespace Engine
