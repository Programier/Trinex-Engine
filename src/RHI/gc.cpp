#include <RHI/gc.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	class RHIGarbageGroup
	{
	public:
		struct Checkpoint
		{
			Checkpoint* next = nullptr;
			RHIQueue* queue  = nullptr;
			u64 id           = 0;

			static Checkpoint* create(RHIGarbageCollector* gc);
		};

		struct Resource
		{
			Resource* next      = nullptr;
			RHIObject* resource = nullptr;
		};

		struct Batch
		{
			Batch* next;
			Resource* resources;

			union
			{
				Checkpoint* checkpoint;
				u64 delay;
			};
		};

	private:
		RHIGarbageGroup* m_next   = nullptr;
		RHIGarbageGroup** m_place = nullptr;
		Batch* m_batch            = nullptr;
		Batch* m_garbage          = nullptr;
		u64 m_delay               = 0;

	private:
		static bool is_relevant(Checkpoint* checkpoint);
		static bool is_completed(Checkpoint* checkpoint);

		static RHIObject* pop(RHIGarbageCollector* gc, Resource*& object);
		static RHIObject* pop(RHIGarbageCollector* gc, Batch*& object);

		static void destroy(RHIGarbageCollector* gc, Checkpoint* object);
		static void destroy(RHIGarbageCollector* gc, Resource* object);
		static void destroy(RHIGarbageCollector* gc, Batch* object, bool with_checkpoints = false);
		static void destroy(RHIGarbageCollector* gc, RHIGarbageGroup* object);

	public:
		RHIGarbageGroup(RHIGarbageGroup** place, u64 delay = 0) : m_next(nullptr), m_place(place), m_delay(delay) {}
		RHIGarbageGroup(RHIGarbageGroup* prev, u64 delay) : m_next(prev->m_next), m_place(&prev->m_next), m_delay(delay)
		{
			prev->m_next = this;

			if (m_next)
			{
				m_next->m_place = &m_next;
			}
		}

		void add(RHIGarbageCollector* gc, RHIObject* object);
		RHIObject* take(RHIGarbageCollector* gc);

		void collect(RHIGarbageCollector* gc, Batch* batch);
		void collect(RHIGarbageCollector* gc);
		void update(RHIGarbageCollector* gc);

		inline RHIGarbageGroup* next() const { return m_next; }
		inline RHIGarbageGroup** place() const { return m_place; }
		friend class RHIGarbageCollector;
	};

	RHIGarbageGroup::Checkpoint* RHIGarbageGroup::Checkpoint::create(RHIGarbageCollector* gc)
	{
		Checkpoint* checkpoint = nullptr;

		for (RHIQueue* queue : gc->rhi()->queues())
		{
			Checkpoint* info = gc->allocate<Checkpoint>();
			info->next       = checkpoint;
			info->queue      = queue;
			info->id         = queue->last_submitted();

			checkpoint = info;
		}

		return checkpoint;
	}

	bool RHIGarbageGroup::is_relevant(Checkpoint* checkpoint)
	{
		while (checkpoint)
		{
			if (checkpoint->id != checkpoint->queue->last_submitted())
			{
				return false;
			}

			checkpoint = checkpoint->next;
		}

		return true;
	}

	bool RHIGarbageGroup::is_completed(Checkpoint* checkpoint)
	{
		while (checkpoint)
		{
			if (checkpoint->id > checkpoint->queue->fence()->value())
			{
				return false;
			}

			checkpoint = checkpoint->next;
		}

		return true;
	}

	RHIObject* RHIGarbageGroup::pop(RHIGarbageCollector* gc, Resource*& object)
	{
		if (object)
		{
			RHIObject* resource = object->resource;

			auto next = object->next;
			gc->free(object);
			object = next;

			if (resource)
			{
				resource->revive();
			}

			return resource;
		}

		return nullptr;
	}

	RHIObject* RHIGarbageGroup::pop(RHIGarbageCollector* gc, Batch*& object)
	{
		RHIObject* resource = pop(gc, object->resources);

		while (object && object->resources == nullptr)
		{
			auto next = object->next;
			gc->free(object);
			object = next;
		}

		return resource;
	}

	void RHIGarbageGroup::destroy(RHIGarbageCollector* gc, Checkpoint* object)
	{
		while (object)
		{
			auto* next = object->next;
			gc->free(object);
			object = next;
		}
	}

	void RHIGarbageGroup::destroy(RHIGarbageCollector* gc, Resource* object)
	{
		while (object)
		{
			trx_delete object->resource;

			auto* next = object->next;
			gc->free(object);
			object = next;
		}
	}

	void RHIGarbageGroup::destroy(RHIGarbageCollector* gc, Batch* object, bool with_checkpoints)
	{
		while (object)
		{
			destroy(gc, object->resources);

			if (with_checkpoints)
			{
				destroy(gc, object->checkpoint);
			}

			auto* next = object->next;
			gc->free(object);
			object = next;
		}
	}

	void RHIGarbageGroup::destroy(RHIGarbageCollector* gc, RHIGarbageGroup* object)
	{
		while (object)
		{
			destroy(gc, object->m_batch, true);
			destroy(gc, object->m_garbage);

			auto* next = object->next();
			trx_delete object;
			object = next;
		}
	}

	void RHIGarbageGroup::add(RHIGarbageCollector* gc, RHIObject* object)
	{
		if (m_batch == nullptr || !is_relevant(m_batch->checkpoint))
		{
			Batch* batch      = gc->allocate<Batch>();
			batch->checkpoint = Checkpoint::create(gc);
			batch->next       = m_batch;
			batch->resources  = nullptr;
			m_batch           = batch;
		}

		Resource* resource = gc->allocate<Resource>();
		resource->next     = m_batch->resources;
		resource->resource = object;

		m_batch->resources = resource;
	}

	RHIObject* RHIGarbageGroup::take(RHIGarbageCollector* gc)
	{
		if (m_garbage)
		{
			return pop(gc, m_garbage);
		}
		else
		{
			collect(gc);

			if (m_garbage)
			{
				return pop(gc, m_garbage);
			}
		}

		return nullptr;
	}

	void RHIGarbageGroup::collect(RHIGarbageCollector* gc, Batch* batch)
	{
		Batch** place = &batch;

		while (Batch* current = *place)
		{
			destroy(gc, current->checkpoint);

			current->delay = m_delay;
			place          = &current->next;
		}

		(*place)  = m_garbage;
		m_garbage = batch;
	}

	void RHIGarbageGroup::collect(RHIGarbageCollector* gc)
	{
		Batch** place = &m_batch;

		while (Batch* batch = *place)
		{
			if (is_completed(batch->checkpoint))
			{
				*place = nullptr;
				return collect(gc, batch);
			}
			else
			{
				place = &batch->next;
			}
		}
	}

	void RHIGarbageGroup::update(RHIGarbageCollector* gc)
	{
		Batch** place = &m_garbage;

		while (Batch* batch = *place)
		{
			if (batch->delay <= 1)
			{
				(*place) = nullptr;
				destroy(gc, batch);
			}
			else
			{
				--batch->delay;
				place = &batch->next;
			}
		}

		collect(gc);
	}

	RHIGarbageCollector::RHIGarbageCollector(RHI* rhi) : m_rhi(rhi)
	{
		m_root = trx_new RHIGarbageGroup(&m_root);
	}

	RHIGarbageCollector::~RHIGarbageCollector()
	{
		m_rhi->wait();
		RHIGarbageGroup::destroy(this, m_root);
	}

	RHIGarbageCollector* RHIGarbageCollector::instance(RHI* rhi)
	{
		return rhi->gc();
	}

	RHIGarbageCollector* RHIGarbageCollector::instance()
	{
		return instance(RHI::instance());
	}

	RHIGarbageGroup* RHIGarbageCollector::create_group(u64 delay)
	{
		return trx_new RHIGarbageGroup(m_root, delay);
	}

	void RHIGarbageCollector::destroy_group(RHIGarbageGroup* group)
	{
		if (group == nullptr)
			return;

		RHIGarbageGroup* next = group->next();
		(*group->place())     = next;

		if (next)
		{
			next->m_place = group->place();
		}

		group->m_next  = nullptr;
		group->m_place = nullptr;
		RHIGarbageGroup::destroy(this, group);
	}

	void RHIGarbageCollector::add(RHIObject* object, RHIGarbageGroup* group)
	{
		if (group == nullptr)
			group = m_root;

		group->add(this, object);
	}

	RHIObject* RHIGarbageCollector::take(RHIGarbageGroup* group)
	{
		if (group == nullptr)
			return nullptr;

		return group->take(this);
	}

	void RHIGarbageCollector::end_frame(u64 frame)
	{
		RHIGarbageGroup* group = m_root;

		while (group)
		{
			group->update(this);
			group = group->next();
		}
	}
} // namespace Trinex
