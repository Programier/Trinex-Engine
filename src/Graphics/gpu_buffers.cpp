#include <Core/archive.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	class NullVertexBuffer : public VertexBufferBase
	{
	public:
		void init()
		{
			render_thread()->call([this]() {
				byte memory[64] = {};
				m_buffer        = rhi->create_buffer(64, m_flags | RHIBufferCreateFlags::VertexBuffer);
				rhi->context()->barrier(m_buffer, RHIAccess::TransferDst);
				rhi->context()->update_buffer(m_buffer, 0, 64, memory);
				rhi->context()->barrier(m_buffer, RHIAccess::VertexBuffer);
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

	VertexBufferBase::VertexBufferBase(RHIBufferCreateFlags type, uint16_t stride, size_t size, const void* data,
	                                   bool keep_cpu_data)
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

	VertexBufferBase& VertexBufferBase::init(RHIBufferCreateFlags type, size_t stride, size_t count, const void* data,
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
			auto create_buffer = [this, keep_cpu_data]() {
				m_buffer = rhi->create_buffer(m_vtx_count * m_stride, m_flags | RHIBufferCreateFlags::VertexBuffer);
				rhi->context()->barrier(m_buffer.get(), RHIAccess::TransferDst);
				rhi->context()->update_buffer(m_buffer.get(), 0, size(), m_data);

				rhi->context()->barrier(m_buffer.get(), RHIAccess::VertexBuffer);
				if (!keep_cpu_data && m_data && !Settings::Rendering::force_keep_cpu_resources)
				{
					ByteAllocator::deallocate(m_data);
					m_data = nullptr;
				}
			};

			if (is_in_render_thread())
			{
				create_buffer();
			}
			else
			{
				render_thread()->call(create_buffer);
			}
		}
		return *this;
	}

	byte* VertexBufferBase::allocate_data(RHIBufferCreateFlags type, uint16_t stride, size_t count)
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

	VertexBufferBase& VertexBufferBase::grow(uint32_t factor)
	{
		factor = Math::max<uint32_t>(factor, 2);

		VertexBufferBase new_buffer;
		byte* ptr = new_buffer.allocate_data(m_flags, m_stride, m_vtx_count * factor);

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

	VertexBufferBase& VertexBufferBase::rhi_update(size_t size, size_t offset)
	{
		if (m_buffer && m_data)
			rhi->context()->update_buffer(m_buffer, offset, size, m_data + offset);
		return *this;
	}

	bool VertexBufferBase::serialize(Archive& ar)
	{
		bool has_data = m_data != nullptr;
		byte flags;
		ar.serialize(flags, m_stride, m_vtx_count, has_data);
		m_flags = RHIBufferCreateFlags::Static;

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

	IndexBuffer::IndexBuffer(RHIBufferCreateFlags type, size_t size, const uint16_t* data, bool keep_cpu_data)
	{
		init(type, size, data, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(RHIBufferCreateFlags type, size_t size, const uint32_t* data, bool keep_cpu_data)
	{
		init(type, size, data, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const std::initializer_list<uint16_t>& list, RHIBufferCreateFlags type, bool keep_cpu_data)
	{
		init(type, list.size(), list.begin(), keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const std::initializer_list<uint32_t>& list, RHIBufferCreateFlags type, bool keep_cpu_data)
	{
		init(type, list.size(), list.begin(), keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const uint16_t* start, const uint16_t* end, RHIBufferCreateFlags type, bool keep_cpu_data)
	{
		init(type, end - start, start, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const uint32_t* start, const uint32_t* end, RHIBufferCreateFlags type, bool keep_cpu_data)
	{
		init(type, end - start, start, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const IndexBuffer& buffer)
	{
		bool keep_cpu_data = buffer.m_buffer && buffer.m_data;

		if (buffer.m_format == RHIIndexFormat::UInt16)
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const uint16_t*>(buffer.m_data), keep_cpu_data);
		}
		else
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const uint32_t*>(buffer.m_data), keep_cpu_data);
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
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const uint16_t*>(buffer.m_data), keep_cpu_data);
		}
		else
		{
			init(buffer.m_flags, buffer.m_idx_count, reinterpret_cast<const uint32_t*>(buffer.m_data), keep_cpu_data);
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

	IndexBuffer& IndexBuffer::init(RHIBufferCreateFlags type, size_t size, const uint16_t* data, bool keep_cpu_data)
	{
		allocate_data(type, RHIIndexFormat::UInt16, size);
		if (data)
			std::memcpy(m_data, data, sizeof(uint16_t) * size);
		return init(keep_cpu_data);
	}

	IndexBuffer& IndexBuffer::init(RHIBufferCreateFlags type, size_t size, const uint32_t* data, bool keep_cpu_data)
	{
		allocate_data(type, RHIIndexFormat::UInt32, size);
		if (data)
			std::memcpy(m_data, data, sizeof(uint32_t) * size);
		return init(keep_cpu_data);
	}

	IndexBuffer& IndexBuffer::init(bool keep_cpu_data)
	{
		if (m_idx_count > 0 && m_buffer == nullptr)
		{
			auto create_buffer = [this, keep_cpu_data]() {
				m_buffer = rhi->create_buffer(size(), m_flags | RHIBufferCreateFlags::IndexBuffer);
				rhi->context()->barrier(m_buffer.get(), RHIAccess::TransferDst);
				rhi->context()->update_buffer(m_buffer, 0, size(), m_data);
				rhi->context()->barrier(m_buffer.get(), RHIAccess::IndexBuffer);

				if (!keep_cpu_data && m_data && !Settings::Rendering::force_keep_cpu_resources)
				{
					ByteAllocator::deallocate(m_data);
					m_data = nullptr;
				}
			};

			if (is_in_render_thread())
			{
				create_buffer();
			}
			else
			{
				render_thread()->call(create_buffer);
			}
		}

		return *this;
	}

	byte* IndexBuffer::allocate_data(RHIBufferCreateFlags type, RHIIndexFormat format, size_t count)
	{
		release();
		m_flags     = type;
		m_format    = format;
		m_idx_count = count;

		m_data = ByteAllocator::allocate_aligned(size(), alignof(uint32_t));
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
		byte flags;
		ar.serialize(m_idx_count, flags, m_format, has_data);
		m_flags = RHIBufferCreateFlags::Static;

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
}// namespace Engine
