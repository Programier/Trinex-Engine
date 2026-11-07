#include <RHI/gc.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	class RHIGarbageGroup
	{
		struct Batch
		{
		};

	private:
		RHIGarbageGroup* m_next;
		u32 m_delay;

	public:
		RHIGarbageGroup(u32 delay = 0) : m_next(nullptr), m_delay(delay) {}
		RHIGarbageGroup(RHIGarbageGroup* prev, u32 delay = 0) : m_next(prev->m_next), m_delay(delay) { prev->m_next = this; }

		void add(RHIObject* object);
		RHIObject* take();
		void update();

		inline u32 delay() const { return m_delay; }
		inline RHIGarbageGroup* next() const { return m_next; }
	};

	void RHIGarbageGroup::add(RHIObject* object) {}

	RHIObject* RHIGarbageGroup::take()
	{
		return nullptr;
	}

	void RHIGarbageGroup::update() {}

	RHIGarbageCollector::RHIGarbageCollector(RHI* rhi)
	{
		m_root = trx_new RHIGarbageGroup();
	}

	RHIGarbageCollector* RHIGarbageCollector::instance(RHI* rhi)
	{
		return rhi->gc();
	}

	RHIGarbageCollector* RHIGarbageCollector::instance()
	{
		return instance(RHI::instance());
	}

	RHIGarbageGroup* RHIGarbageCollector::create_group(u32 delay)
	{
		return trx_new RHIGarbageGroup(m_root, delay);
	}

	void RHIGarbageCollector::add(RHIObject* object, RHIGarbageGroup* group)
	{
		if (group == nullptr)
			group = m_root;

		group->add(object);
	}

	RHIObject* RHIGarbageCollector::take(RHIGarbageGroup* group)
	{
		if (group == nullptr)
			return nullptr;

		return group->take();
	}

	void RHIGarbageCollector::update()
	{
		RHIGarbageGroup* group = m_root;

		while (group)
		{
			group->update();
			group = group->next();
		}
	}

} // namespace Trinex
