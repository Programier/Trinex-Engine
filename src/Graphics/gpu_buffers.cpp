#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	class NullVertexBuffer : public VertexBufferBase
	{
	public:
		void init()
		{
			m_buffer = RHI::instance()->create_buffer(64, m_flags | RHIBufferFlags::VertexBuffer);

			RHIContextPool::global_instance()->execute([this](RHIContext* ctx) {
				u8 memory[64] = {};
				ctx->barrier(m_buffer, RHIAccess::TransferDst);
				ctx->update(m_buffer, memory, {.size = 64});
				ctx->barrier(m_buffer, RHIAccess::VertexBuffer);
			});
		}

	} s_null_vertex_buffer;

	static StartupResourcesInitializeController on_resource_init([]() { s_null_vertex_buffer.init(); });
	static DestroyController on_resource_destroy([]() { s_null_vertex_buffer.release(); });

	const VertexBufferBase* VertexBufferBase::static_null()
	{
		return &s_null_vertex_buffer;
	}

	VertexBufferBase::VertexBufferBase() = default;

	VertexBufferBase::~VertexBufferBase()
	{
		if (m_data)
		{
			ByteAllocator::deallocate(m_data);
		}
	}

	VertexBufferBase::VertexBufferBase(RHIBufferFlags type, u16 stride, usize size, const void* data, bool keep_cpu_data)
	{
		init(type, stride, size, data, keep_cpu_data);
	}

	VertexBufferBase::VertexBufferBase(const VertexBufferBase& buffer)
	{
		bool keep_cpu_data = buffer.m_buffer && buffer.m_data;
		init(buffer.m_flags, buffer.m_stride, buffer.m_vtx_count, buffer.m_data, keep_cpu_data);
	}

	VertexBufferBase::VertexBufferBase(VertexBufferBase&& buffer)
	{
		m_flags     = buffer.m_flags;
		m_vtx_count = buffer.m_vtx_count;
		m_stride    = buffer.m_stride;
		m_data      = buffer.m_data;
		m_buffer    = std::move(buffer.m_buffer);

		buffer.m_vtx_count = 0;
		buffer.m_stride    = 0;
		buffer.m_data      = nullptr;
		buffer.m_flags     = RHIBufferFlags::Static;
	}

	VertexBufferBase& VertexBufferBase::operator=(const VertexBufferBase& buffer)
	{
		if (this == &buffer)
			return *this;

		return init(buffer.m_flags, buffer.m_stride, buffer.m_vtx_count, buffer.m_data);
	}

	VertexBufferBase& VertexBufferBase::operator=(VertexBufferBase&& buffer)
	{
		if (this == &buffer)
			return *this;

		m_flags     = buffer.m_flags;
		m_vtx_count = buffer.m_vtx_count;
		m_stride    = buffer.m_stride;
		m_data      = buffer.m_data;
		m_buffer    = std::move(buffer.m_buffer);

		buffer.m_vtx_count = 0;
		buffer.m_stride    = 0;
		buffer.m_data      = nullptr;
		return *this;
	}

	VertexBufferBase& VertexBufferBase::init(RHIBufferFlags type, usize stride, usize count, const void* data,
	                                         bool keep_cpu_data)
	{
		allocate_data(type, stride, count);
		if (data)
			std::memcpy(m_data, data, count * stride);
		return init(keep_cpu_data);
	}

	VertexBufferBase& VertexBufferBase::init(bool keep_cpu_data)
	{
		if (m_vtx_count > 0 && m_buffer == nullptr)
		{
			m_buffer = RHI::instance()->create_buffer(m_vtx_count * m_stride, m_flags | RHIBufferFlags::VertexBuffer);

			RHIContextPool::global_instance()->execute([this](RHIContext* ctx) {
				ctx->barrier(m_buffer.get(), RHIAccess::TransferDst);
				ctx->update(m_buffer.get(), m_data, {.size = size()});
				ctx->barrier(m_buffer.get(), RHIAccess::VertexBuffer);
			});

			if (!keep_cpu_data && m_data && !Settings::Rendering::force_keep_cpu_resources)
			{
				ByteAllocator::deallocate(m_data);
				m_data = nullptr;
			}
		}
		return *this;
	}

	u8* VertexBufferBase::allocate_data(RHIBufferFlags type, u16 stride, usize count)
	{
		release();
		m_flags     = type;
		m_stride    = stride;
		m_vtx_count = count;
		m_data      = ByteAllocator::allocate(count * stride);
		return m_data;
	}

	VertexBufferBase& VertexBufferBase::release()
	{
		if (m_data)
		{
			ByteAllocator::deallocate(m_data);
			m_data = nullptr;
		}

		m_vtx_count = 0;
		m_stride    = 0;
		m_buffer    = nullptr;
		return *this;
	}

	VertexBufferBase& VertexBufferBase::grow(u32 factor)
	{
		factor = Math::max<u32>(factor, 2);

		VertexBufferBase new_buffer;
		u8* ptr = new_buffer.allocate_data(m_flags, m_stride, m_vtx_count * factor);

		const bool keep_cpu_data       = m_data != nullptr;
		const bool need_initialization = m_buffer != nullptr;

		if (keep_cpu_data)
			std::memcpy(ptr, m_data, size());

		(*this) = std::move(new_buffer);

		if (need_initialization)
		{
			init(keep_cpu_data);
		}
		return *this;
	}

	VertexBufferBase& VertexBufferBase::rhi_update(class RHIContext* ctx, usize size, usize offset)
	{
		if (m_buffer && m_data)
			ctx->update(m_buffer, m_data, {.size = size, .dst_offset = offset, .src_offset = offset});
		return *this;
	}

	bool VertexBufferBase::serialize(Archive& ar)
	{
		bool has_data = m_data != nullptr;
		u8 flags;
		ar.serialize(flags, m_stride, m_vtx_count, has_data);
		m_flags = RHIBufferFlags::Static;

		if (has_data)
		{
			if (ar.is_reading())
			{
				allocate_data(m_flags, m_stride, m_vtx_count);
			}

			ar.serialize_memory(m_data, size());
		}
		return ar;
	}

	IndexBuffer::IndexBuffer() = default;

	IndexBuffer::~IndexBuffer()
	{
		if (m_data)
		{
			ByteAllocator::deallocate(m_data);
		}
	}

	IndexBuffer::IndexBuffer(RHIBufferFlags type, usize size, const u16* data, bool keep_cpu_data)
	{
		init(type, size, data, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(RHIBufferFlags type, usize size, const u32* data, bool keep_cpu_data)
	{
		init(type, size, data, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const std::initializer_list<u16>& list, RHIBufferFlags type, bool keep_cpu_data)
	{
		init(type, list.size(), list.begin(), keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const std::initializer_list<u32>& list, RHIBufferFlags type, bool keep_cpu_data)
	{
		init(type, list.size(), list.begin(), keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const u16* start, const u16* end, RHIBufferFlags type, bool keep_cpu_data)
	{
		init(type, end - start, start, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const u32* start, const u32* end, RHIBufferFlags type, bool keep_cpu_data)
	{
		init(type, end - start, start, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const IndexBuffer& buffer)
	{
		bool keep_cpu_data = buffer.m_buffer && buffer.m_data;

		if (buffer.m_format == RHIIndexFormat::UInt16)
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const u16*>(buffer.m_data), keep_cpu_data);
		}
		else
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const u32*>(buffer.m_data), keep_cpu_data);
		}
	}

	IndexBuffer::IndexBuffer(IndexBuffer&& buffer)
	{
		m_flags     = buffer.m_flags;
		m_format    = buffer.m_format;
		m_idx_count = buffer.m_idx_count;
		m_data      = buffer.m_data;
		m_buffer    = std::move(buffer.m_buffer);

		buffer.m_idx_count = 0;
		buffer.m_data      = nullptr;
	}

	IndexBuffer& IndexBuffer::operator=(const IndexBuffer& buffer)
	{
		if (this == &buffer)
			return *this;

		bool keep_cpu_data = buffer.m_buffer && buffer.m_data;

		if (buffer.m_format == RHIIndexFormat::UInt16)
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const u16*>(buffer.m_data), keep_cpu_data);
		}
		else
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const u32*>(buffer.m_data), keep_cpu_data);
		}
		return *this;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& buffer)
	{
		if (this == &buffer)
			return *this;

		m_flags     = buffer.m_flags;
		m_format    = buffer.m_format;
		m_idx_count = buffer.m_idx_count;
		m_data      = buffer.m_data;
		m_buffer    = std::move(buffer.m_buffer);

		buffer.m_idx_count = 0;
		buffer.m_data      = nullptr;
		return *this;
	}

	IndexBuffer& IndexBuffer::init(RHIBufferFlags type, usize size, const u16* data, bool keep_cpu_data)
	{
		allocate_data(type, RHIIndexFormat::UInt16, size);
		if (data)
			std::memcpy(m_data, data, sizeof(u16) * size);
		return init(keep_cpu_data);
	}

	IndexBuffer& IndexBuffer::init(RHIBufferFlags type, usize size, const u32* data, bool keep_cpu_data)
	{
		allocate_data(type, RHIIndexFormat::UInt32, size);
		if (data)
			std::memcpy(m_data, data, sizeof(u32) * size);
		return init(keep_cpu_data);
	}

	IndexBuffer& IndexBuffer::init(bool keep_cpu_data)
	{
		if (m_idx_count > 0 && m_buffer == nullptr)
		{
			m_buffer = RHI::instance()->create_buffer(size(), m_flags | RHIBufferFlags::IndexBuffer);

			RHIContextPool::global_instance()->execute([this](RHIContext* ctx) {
				ctx->barrier(m_buffer.get(), RHIAccess::TransferDst);
				ctx->update(m_buffer.get(), m_data, {.size = size()});
				ctx->barrier(m_buffer.get(), RHIAccess::IndexBuffer);
			});

			if (!keep_cpu_data && m_data && !Settings::Rendering::force_keep_cpu_resources)
			{
				ByteAllocator::deallocate(m_data);
				m_data = nullptr;
			}
		}

		return *this;
	}

	u8* IndexBuffer::allocate_data(RHIBufferFlags type, RHIIndexFormat format, usize count)
	{
		release();
		m_flags     = type;
		m_format    = format;
		m_idx_count = count;

		m_data = ByteAllocator::allocate_aligned(size(), alignof(u32));
		return m_data;
	}

	IndexBuffer& IndexBuffer::release()
	{
		if (m_data)
		{
			ByteAllocator::deallocate(m_data);
			m_data = nullptr;
		}

		m_idx_count = 0;
		m_buffer    = nullptr;
		return *this;
	}

	bool IndexBuffer::serialize(Archive& ar)
	{
		bool has_data = m_data != nullptr;
		u8 flags;
		ar.serialize(m_idx_count, flags, m_format, has_data);
		m_flags = RHIBufferFlags::Static;

		if (has_data)
		{
			if (ar.is_reading())
			{
				allocate_data(m_flags, m_format, m_idx_count);
			}

			ar.serialize_memory(m_data, size());
		}
		return ar;
	}
}// namespace Trinex
