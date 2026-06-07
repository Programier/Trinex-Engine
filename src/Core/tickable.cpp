#include <Core/threading.hpp>
#include <Core/tickable.hpp>

namespace Trinex
{
	trinex_implement_registry(Tickable);

	void Tickable::for_each_begin_frame()
	{
		Tickable* head = static_first();

		while (head)
		{
			if (head->is_tickable())
			{
				head->begin_frame();
			}

			head = head->next();
		}
	}

	void Tickable::for_each_update(float dt)
	{
		Tickable* head = static_first();

		while (head)
		{
			if (head->is_tickable())
			{
				head->update(dt);
			}

			head = head->next();
		}
	}

	void Tickable::for_each_end_frame()
	{
		Tickable* head = static_first();

		while (head)
		{
			if (head->is_tickable())
			{
				head->end_frame();
			}

			head = head->next();
		}
	}

	Tickable& Tickable::begin_frame()
	{
		return *this;
	}

	Tickable& Tickable::update(float dt)
	{
		return *this;
	}

	Tickable& Tickable::end_frame()
	{
		return *this;
	}

	bool Tickable::is_tickable() const
	{
		return true;
	}

	trinex_implement_registry(ThreadLocalTickable);

	ThreadLocalTickable::ThreadLocalTickable() : m_thread(Thread::static_self()) {}

	void ThreadLocalTickable::for_each_begin_frame()
	{
		ThreadLocalTickable* head = static_first();
	}

	void ThreadLocalTickable::for_each_update(float dt) {}

	void ThreadLocalTickable::for_each_end_frame() {}

	ThreadLocalTickable& ThreadLocalTickable::begin_frame()
	{
		return *this;
	}

	ThreadLocalTickable& ThreadLocalTickable::update(float dt)
	{
		return *this;
	}

	ThreadLocalTickable& ThreadLocalTickable::end_frame()
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
		Tickable& begin_frame() override
		{
			ThreadLocalTickable::for_each_begin_frame();
			return *this;
		}

		Tickable& update(float dt) override
		{
			ThreadLocalTickable::for_each_update(dt);
			return *this;
		}

		Tickable& end_frame() override
		{
			ThreadLocalTickable::for_each_end_frame();
			return *this;
		}
	} s_tickable;
}// namespace Trinex
