#pragma once
#include <Core/etl/registry.hpp>

namespace Trinex
{
	class Thread;

	class trinex_api Tickable : public Registry<Tickable>
	{
		trinex_registry(Tickable);

	public:
		static void for_each_begin_frame(u64 frame);
		static void for_each_update(f32 dt);
		static void for_each_end_frame(u64 frame);

		virtual Tickable& begin_frame(u64 frame);
		virtual Tickable& update(f32 dt);
		virtual Tickable& end_frame(u64 frame);
		virtual bool is_tickable() const;
	};

	class trinex_api ThreadLocalTickable : public Registry<ThreadLocalTickable>
	{
		trinex_registry(ThreadLocalTickable);

	private:
		Thread* m_thread = nullptr;

	public:
		class trinex_api CriticalSection final
		{
		public:
			CriticalSection();
			~CriticalSection();

			trinex_non_copyable(CriticalSection);
			trinex_non_moveable(CriticalSection);
		};

		ThreadLocalTickable();

		static void for_each_begin_frame(u64 frame);
		static void for_each_update(f32 dt);
		static void for_each_end_frame(u64 frame);
		static inline CriticalSection critical_section() { return {}; }

		virtual ThreadLocalTickable& begin_frame(u64 frame);
		virtual ThreadLocalTickable& update(f32 dt);
		virtual ThreadLocalTickable& end_frame(u64 frame);
		virtual bool is_tickable() const;

		inline Thread* thread() const { return m_thread; }
	};
} // namespace Trinex
