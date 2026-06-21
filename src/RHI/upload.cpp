#include <Core/etl/algorithm.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/lifecycle.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/tickable.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>
#include <RHI/upload.hpp>

namespace Trinex
{
	static class RHIUploadAllocatorImpl* g_allocator = nullptr;

	class RHIUploadAllocatorPage final : public RHIObject
	{
	public:
		RHIUploadAllocatorPage* next = nullptr;

	private:
		RHIBuffer* m_buffer = nullptr;
		u8* m_mapped        = nullptr;
		usize m_capacity    = 0;
		usize m_offset      = 0;

	public:
		RHIUploadAllocatorPage(usize capacity) : m_capacity(capacity)
		{
			m_buffer = RHI::instance()->create_buffer(capacity, RHIBufferFlags::TransferSrc | RHIBufferFlags::CPUWrite);
			m_mapped = m_buffer ? m_buffer->map(RHIMappingAccess::Write) : nullptr;
			trinex_assert_msg(m_mapped, "Failed to map upload page");
		}

		void destroy() override;
		inline void reset() { m_offset = 0; }

		inline bool has_space(usize size, usize alignment) const
		{
			const usize aligned_offset = align_up(m_offset, alignment);
			return aligned_offset + size <= m_capacity;
		}

		inline RHIUploadAllocation allocate(usize size, usize alignment)
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

		~RHIUploadAllocatorPage() override
		{
			if (m_buffer)
			{
				m_buffer->unmap();
				m_buffer->release();
			}
		}
	};

	class RHIUploadAllocatorImpl : public ThreadLocalTickable
	{
	public:
		static constexpr usize page_size = 8 * 1024 * 1024;// 8 MB

	private:
		Trinex::CriticalSection m_critical;
		Vector<RHIUploadAllocatorPage*> m_pages;
		RHIUploadAllocatorPage* m_page = nullptr;
		RHIUploadAllocatorPage* m_free = nullptr;

	public:
		RHIUploadAllocatorPage* request_page(usize size)
		{
			RHIUploadAllocatorPage** page_ptr = &m_free;
			RHIUploadAllocatorPage* page;

			while ((page = (*page_ptr)))
			{
				if (page->capacity() >= size)
				{
					(*page_ptr) = page->next;
					return page;
				}

				page_ptr = &page->next;
			}

			page = trx_new RHIUploadAllocatorPage(align_up(size, page_size));
			m_pages.push_back(page);
			return page;
		}


		void release_page(RHIUploadAllocatorPage* page)
		{
			if (page)
			{
				ScopeLock lock(m_critical);
				page->reset();
				page->next = m_free;
				m_free     = page;
			}
		}

		RHIUploadAllocation allocate(RHIContext* context, usize size, usize alignment = 16)
		{
			trinex_assert_msg(context, "Upload allocation requires a valid RHI context");
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

		RHIUploadAllocation allocate_chunk(RHIContext* context, usize max_size, usize alignment = 16)
		{
			trinex_assert_msg(context, "Upload allocation requires a valid RHI context");
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

		RHIUploadAllocatorStats statistics()
		{
			ScopeLock lock(m_critical);

			RHIUploadAllocatorStats stats;
			stats.total_pages = m_pages.size();

			for (RHIUploadAllocatorPage* page : m_pages)
			{
				stats.total_capacity += page->capacity();
			}

			for (RHIUploadAllocatorPage* page = m_free; page; page = page->next)
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

		RHIUploadAllocatorImpl& update(float) override
		{
			ScopeLock lock(m_critical);
			return *this;
		}

		~RHIUploadAllocatorImpl()
		{
			ScopeLock lock(m_critical);
			g_allocator = nullptr;

			for (auto* page : m_pages)
			{
				page->release();
			}
		}
	};

	static RHIUploadAllocatorImpl* allocator_instance()
	{
		static struct Initializer {
			Initializer()
			{
				g_allocator = trx_new RHIUploadAllocatorImpl();

				LifeCycle::on_shutdown([]() {
					trx_delete g_allocator;
					g_allocator = nullptr;
				});
			}
		} initializer;

		return g_allocator;
	}

	void RHIUploadAllocatorPage::destroy()
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

	RHIUploadAllocation RHIUploadAllocator::allocate(RHIContext* context, usize size, usize alignment)
	{
		return allocator_instance()->allocate(context, size, alignment);
	}

	RHIUploadAllocation RHIUploadAllocator::allocate_chunk(RHIContext* context, usize max_size, usize alignment)
	{
		return allocator_instance()->allocate_chunk(context, max_size, alignment);
	}

	RHIUploadAllocatorStats RHIUploadAllocator::statistics()
	{
		if (g_allocator == nullptr)
			return {};

		return g_allocator->statistics();
	}

	RHIContext& RHIContext::update(RHIBuffer* dst, const void* src, const RHIBufferCopy& region)
	{
		const u8* data   = static_cast<const u8*>(src) + region.src_offset;
		usize remaining  = region.size;
		usize src_offset = 0;
		usize dst_offset = region.dst_offset;

		while (remaining > 0)
		{
			auto upload = RHIUploadAllocator::allocate_chunk(this, remaining, 16);
			std::memcpy(upload.data, data + src_offset, upload.size);

			copy(dst, upload.buffer, {.size = upload.size, .dst_offset = dst_offset, .src_offset = upload.offset});

			remaining -= upload.size;
			src_offset += upload.size;
			dst_offset += upload.size;
		}

		return *this;
	}

	RHIContext& RHIContext::update(RHITexture* dst, const RHITextureRegion& dst_region, const void* src,
	                               const RHIBufferTextureCopy& src_region)
	{
		auto upload = RHIUploadAllocator::allocate(this, src_region.size, 16);
		std::memcpy(upload.data, static_cast<const u8*>(src) + src_region.offset, src_region.size);

		RHIBufferTextureCopy copy_region = src_region;
		copy_region.offset               = upload.offset;
		return copy(dst, dst_region, upload.buffer, copy_region);
	}
}// namespace Trinex
