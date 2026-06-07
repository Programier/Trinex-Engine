#include <Core/etl/critical_section.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>

namespace Trinex
{
	namespace
	{
		template<typename Callback>
		void for_each_thread_local_tickable(Callback&& callback)
		{
			Thread* self = Thread::static_self();

			ThreadLocalTickable::for_each([&](ThreadLocalTickable* current) {
				if (!current->is_tickable())
					return;

				Thread* target_thread = current->thread();

				if (target_thread == nullptr || target_thread == self)
				{
					callback(current);
				}
				else
				{
					target_thread->add_task(Task(Task::High, [current, callback]() mutable { callback(current); }));
				}
			});
		}
	}// namespace

	trinex_implement_registry(Tickable);

	void Tickable::for_each_begin_frame(u64 frame)
	{
		Tickable::for_each([frame](Tickable* head) {
			if (head->is_tickable())
			{
				head->begin_frame(frame);
			}
		});
	}

	void Tickable::for_each_update(f32 dt)
	{
		Tickable::for_each([dt](Tickable* head) {
			if (head->is_tickable())
			{
				head->update(dt);
			}
		});
	}

	void Tickable::for_each_end_frame(u64 frame)
	{
		Tickable::for_each([frame](Tickable* head) {
			if (head->is_tickable())
			{
				head->end_frame(frame);
			}
		});
	}

	Tickable& Tickable::begin_frame(u64 frame)
	{
		return *this;
	}

	Tickable& Tickable::update(float dt)
	{
		return *this;
	}

	Tickable& Tickable::end_frame(u64 frame)
	{
		return *this;
	}

	bool Tickable::is_tickable() const
	{
		return true;
	}

	trinex_implement_registry(ThreadLocalTickable);

	static CriticalSectionRecursive* thread_local_critical_section()
	{
		static CriticalSectionRecursive section;
		return &section;
	}

	ThreadLocalTickable::CriticalSection::CriticalSection()
	{
		thread_local_critical_section()->lock();
	}

	ThreadLocalTickable::CriticalSection::~CriticalSection()
	{
		thread_local_critical_section()->unlock();
	}

	ThreadLocalTickable::ThreadLocalTickable() : m_thread(Thread::static_self()) {}

	void ThreadLocalTickable::for_each_begin_frame(u64 frame)
	{
		for_each_thread_local_tickable([frame](ThreadLocalTickable* tickable) { tickable->begin_frame(frame); });
	}

	void ThreadLocalTickable::for_each_update(float dt)
	{
		for_each_thread_local_tickable([dt](ThreadLocalTickable* tickable) { tickable->update(dt); });
	}

	void ThreadLocalTickable::for_each_end_frame(u64 frame)
	{
		for_each_thread_local_tickable([frame](ThreadLocalTickable* tickable) { tickable->end_frame(frame); });
	}

	ThreadLocalTickable& ThreadLocalTickable::begin_frame(u64 frame)
	{
		return *this;
	}

	ThreadLocalTickable& ThreadLocalTickable::update(float dt)
	{
		return *this;
	}

	ThreadLocalTickable& ThreadLocalTickable::end_frame(u64 frame)
	{
		return *this;
	}

	bool ThreadLocalTickable::is_tickable() const
	{
		return true;
	}


	static class : public Tickable
	{
	public:
		Tickable& begin_frame(u64 frame) override
		{
			ThreadLocalTickable::for_each_begin_frame(frame);
			return *this;
		}

		Tickable& update(float dt) override
		{
			ThreadLocalTickable::for_each_update(dt);
			return *this;
		}

		Tickable& end_frame(u64 frame) override
		{
			ThreadLocalTickable::for_each_end_frame(frame);
			return *this;
		}
	} s_tickable;
}// namespace Trinex
