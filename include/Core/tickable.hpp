#pragma once
#include <Core/etl/registry.hpp>

namespace Trinex
{
	class Thread;

	class ENGINE_EXPORT Tickable : public Registry<Tickable>
	{
		trinex_registry(Tickable);

	public:
		static void for_each_begin_frame();
		static void for_each_update(float dt);
		static void for_each_end_frame();

		virtual Tickable& begin_frame();
		virtual Tickable& update(float dt);
		virtual Tickable& end_frame();
		virtual bool is_tickable() const;
	};

	class ENGINE_EXPORT ThreadLocalTickable : public Registry<ThreadLocalTickable>
	{
		trinex_registry(ThreadLocalTickable);

	private:
		Thread* m_thread = nullptr;

	public:
		ThreadLocalTickable();

		static void for_each_begin_frame();
		static void for_each_update(float dt);
		static void for_each_end_frame();

		virtual ThreadLocalTickable& begin_frame();
		virtual ThreadLocalTickable& update(float dt);
		virtual ThreadLocalTickable& end_frame();
		virtual bool is_tickable() const;

		inline Thread* thread() const { return m_thread; }
	};
}// namespace Trinex
