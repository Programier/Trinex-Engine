#include <Core/etl/algorithm.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/vector.hpp>
#include <Core/lifecycle.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/tickable.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/readback.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	static class RHIReadBackAllocatorImpl* g_allocator = nullptr;

	class RHIReadBackAllocatorPage final : public RHIObject
	{
	public:
		RHIReadBackAllocatorPage* next = nullptr;

	private:
		RHIBuffer* m_buffer = nullptr;
		u8* m_mapped        = nullptr;
		usize m_capacity    = 0;
		usize m_offset      = 0;

	public:
		RHIReadBackAllocatorPage(usize capacity) : m_capacity(capacity)
		{
			m_buffer = RHI::instance()->create_buffer(capacity, RHIBufferFlags::TransferDst | RHIBufferFlags::CPURead);
			m_mapped = m_buffer ? m_buffer->map(RHIMappingAccess::Read) : nullptr;
			trinex_assert_msg(m_mapped, "Failed to map readback page");
		}

		void destroy() override;
		inline void reset() { m_offset = 0; }

		inline bool has_space(usize size, usize alignment) const
		{
			const usize aligned_offset = align_up(m_offset, alignment);
			return aligned_offset + size <= m_capacity;
		}

		inline RHIReadBackAllocation allocate(usize size, usize alignment)
		{
			const usize aligned_offset = align_up(m_offset, alignment);
			m_offset                   = aligned_offset + size;
			return {.buffer = m_buffer, .data = m_mapped + aligned_offset, .offset = aligned_offset, .size = size};
		}

		inline usize capacity() const { return m_capacity; }
		inline usize used() const { return m_offset; }
		inline usize free() const { return m_capacity - m_offset; }
		inline usize available(usize alignment) const
		{
			const usize aligned_offset = align_up(m_offset, alignment);
			return aligned_offset < m_capacity ? m_capacity - aligned_offset : 0;
		}

		~RHIReadBackAllocatorPage() override
		{
			if (m_buffer)
			{
				m_buffer->unmap();
				m_buffer->release();
			}
		}
	};

	class RHIReadBackAllocatorImpl : public ThreadLocalTickable
	{
	public:
		static constexpr usize page_size = 8 * 1024 * 1024;// 8 MB

	private:
		Trinex::CriticalSection m_critical;
		Vector<RHIReadBackAllocatorPage*> m_pages;
		RHIReadBackAllocatorPage* m_page = nullptr;
		RHIReadBackAllocatorPage* m_free = nullptr;

	public:
		RHIReadBackAllocatorPage* request_page(usize size)
		{
			RHIReadBackAllocatorPage** page_ptr = &m_free;
			RHIReadBackAllocatorPage* page;

			while ((page = (*page_ptr)))
			{
				if (page->capacity() >= size)
				{
					(*page_ptr) = page->next;
					return page;
				}

				page_ptr = &page->next;
			}

			page = trx_new RHIReadBackAllocatorPage(align_up(size, page_size));
			m_pages.push_back(page);
			return page;
		}

		void release_page(RHIReadBackAllocatorPage* page)
		{
			if (page)
			{
				ScopeLock lock(m_critical);
				page->reset();
				page->next = m_free;
				m_free     = page;
			}
		}

		RHIReadBackAllocation allocate(RHIContext* context, usize size, usize alignment = 16)
		{
			trinex_assert_msg(context, "Readback allocation requires a valid RHI context");
			ScopeLock lock(m_critical);

			if (m_page == nullptr)
			{
				m_page = request_page(size);
			}
			else if (!m_page->has_space(size, alignment))
			{
				m_page->release();
				m_page = request_page(size);
			}

			context->track_resource(m_page);
			return m_page->allocate(size, alignment);
		}

		RHIReadBackAllocation allocate_chunk(RHIContext* context, usize max_size, usize alignment = 16)
		{
			trinex_assert_msg(context, "Readback allocation requires a valid RHI context");
			ScopeLock lock(m_critical);

			if (m_page == nullptr)
			{
				m_page = request_page(Math::min(max_size, page_size));
			}

			usize available = m_page->available(alignment);

			if (available == 0)
			{
				m_page->release();
				m_page    = request_page(Math::min(max_size, page_size));
				available = m_page->available(alignment);
			}

			const usize size = Math::min(max_size, available);
			context->track_resource(m_page);
			return m_page->allocate(size, alignment);
		}

		RHIReadBackAllocatorStats statistics()
		{
			ScopeLock lock(m_critical);

			RHIReadBackAllocatorStats stats;
			stats.total_pages = m_pages.size();

			for (RHIReadBackAllocatorPage* page : m_pages)
			{
				stats.total_capacity += page->capacity();
			}

			for (RHIReadBackAllocatorPage* page = m_free; page; page = page->next)
			{
				++stats.free_pages;
				stats.free_capacity += page->capacity();
			}

			stats.used_pages    = stats.total_pages - stats.free_pages;
			stats.used_capacity = stats.total_capacity - stats.free_capacity;

			if (m_page)
			{
				stats.current_page_used     = m_page->used();
				stats.current_page_free     = m_page->free();
				stats.current_page_capacity = m_page->capacity();
			}

			return stats;
		}

		RHIReadBackAllocatorImpl& update(float) override
		{
			ScopeLock lock(m_critical);
			return *this;
		}

		~RHIReadBackAllocatorImpl()
		{
			ScopeLock lock(m_critical);
			g_allocator = nullptr;

			for (auto* page : m_pages)
			{
				page->release();
			}
		}
	};

	static RHIReadBackAllocatorImpl* allocator_instance()
	{
		static struct Initializer {
			Initializer()
			{
				g_allocator = trx_new RHIReadBackAllocatorImpl();

				LifeCycle::on_shutdown([]() {
					trx_delete g_allocator;
					g_allocator = nullptr;
				});
			}
		} initializer;

		return g_allocator;
	}

	void RHIReadBackAllocatorPage::destroy()
	{
		if (g_allocator)
		{
			add_reference();
			g_allocator->release_page(this);
		}
		else
		{
			trx_delete this;
		}
	}

	RHIReadBackAllocation RHIReadBackAllocator::allocate(RHIContext* context, usize size, usize alignment)
	{
		return allocator_instance()->allocate(context, size, alignment);
	}

	RHIReadBackAllocation RHIReadBackAllocator::allocate_chunk(RHIContext* context, usize max_size, usize alignment)
	{
		return allocator_instance()->allocate_chunk(context, max_size, alignment);
	}

	RHIReadBackAllocatorStats RHIReadBackAllocator::statistics()
	{
		if (g_allocator == nullptr)
			return {};

		return g_allocator->statistics();
	}
}// namespace Trinex
