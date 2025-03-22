#include <Core/etl/allocator.hpp>
#include <Core/memory.hpp>
#include <Core/threading.hpp>
#include <Engine/settings.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	VertexBufferBase::VertexBufferBase() = default;

	VertexBufferBase::VertexBufferBase(RHIBufferType type, uint16_t stride, size_t size, const void* data, bool keep_cpu_data)
	{
		init(type, stride, size, data, keep_cpu_data);
	}

	VertexBufferBase::VertexBufferBase(const VertexBufferBase& buffer)
	{
		bool keep_cpu_data = buffer.m_buffer && buffer.m_data;
		init(buffer.m_type, buffer.m_stride, buffer.m_vtx_count, buffer.m_data, keep_cpu_data);
	}

	VertexBufferBase::VertexBufferBase(VertexBufferBase&& buffer)
	{
		m_type      = buffer.m_type;
		m_vtx_count = buffer.m_vtx_count;
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

		return init(buffer.m_type, buffer.m_stride, buffer.m_vtx_count, buffer.m_data);
	}

	VertexBufferBase& VertexBufferBase::operator=(VertexBufferBase&& buffer)
	{
		if (this == &buffer)
			return *this;

		m_type      = buffer.m_type;
		m_vtx_count = buffer.m_vtx_count;
		m_data      = buffer.m_data;
		m_buffer    = std::move(buffer.m_buffer);

		buffer.m_vtx_count = 0;
		buffer.m_stride    = 0;
		buffer.m_data      = nullptr;
		return *this;
	}

	VertexBufferBase& VertexBufferBase::init(RHIBufferType type, size_t stride, size_t count, const void* data,
											 bool keep_cpu_data)
	{
		allocate_data(type, stride, count);
		std::memcpy(m_data, data, count * stride);
		return init(keep_cpu_data);
	}

	VertexBufferBase& VertexBufferBase::init(bool keep_cpu_data)
	{
		if (m_vtx_count > 0 && m_buffer == nullptr)
		{
			auto create_buffer = [this, keep_cpu_data]() {
				m_buffer = rhi->create_vertex_buffer(m_vtx_count * m_stride, m_data, m_type);
				if (!keep_cpu_data && m_data && !Settings::Rendering::force_keep_cpu_resources)
				{
					release_memory(m_data);
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

	byte* VertexBufferBase::allocate_data(RHIBufferType type, uint16_t stride, size_t count)
	{
		release();
		m_type      = type;
		m_stride    = stride;
		m_vtx_count = count;
		m_data      = allocate_memory(count * stride);
		return m_data;
	}

	VertexBufferBase& VertexBufferBase::release()
	{
		if (m_data)
		{
			release_memory(m_data);
			m_data = nullptr;
		}

		m_vtx_count = 0;
		m_stride    = 0;
		m_buffer    = nullptr;
		return *this;
	}

	VertexBufferBase& VertexBufferBase::rhi_bind(byte stream, size_t offset)
	{
		if (m_buffer)
		{
			m_buffer->bind(stream, m_stride, offset);
		}
		return *this;
	}

	VertexBufferBase& VertexBufferBase::rhi_update(size_t size, size_t offset)
	{
		if (m_buffer && m_data)
		{
			m_buffer->update(offset, size, m_data + offset);
		}
		return *this;
	}

	bool VertexBufferBase::serialize(Archive& ar)
	{
		return ar;
	}

	IndexBuffer::IndexBuffer() = default;

	IndexBuffer::IndexBuffer(RHIBufferType type, size_t size, const uint16_t* data, bool keep_cpu_data)
	{
		init(type, size, data, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(RHIBufferType type, size_t size, const uint32_t* data, bool keep_cpu_data)
	{
		init(type, size, data, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const std::initializer_list<uint16_t>& list, RHIBufferType type, bool keep_cpu_data)
	{
		init(type, list.size(), list.begin(), keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const std::initializer_list<uint32_t>& list, RHIBufferType type, bool keep_cpu_data)
	{
		init(type, list.size(), list.begin(), keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const uint16_t* start, const uint16_t* end, RHIBufferType type, bool keep_cpu_data)
	{
		init(type, end - start, start, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const uint32_t* start, const uint32_t* end, RHIBufferType type, bool keep_cpu_data)
	{
		init(type, end - start, start, keep_cpu_data);
	}

	IndexBuffer::IndexBuffer(const IndexBuffer& buffer)
	{
		bool keep_cpu_data = buffer.m_buffer && buffer.m_data;

		if (buffer.m_format == RHIIndexFormat::UInt16)
		{
			init(buffer.m_type, buffer.m_idx_count, reinterpret_cast<const uint16_t*>(buffer.m_data), keep_cpu_data);
		}
		else
		{
			init(buffer.m_type, buffer.m_idx_count, reinterpret_cast<const uint32_t*>(buffer.m_data), keep_cpu_data);
		}
	}

	IndexBuffer::IndexBuffer(IndexBuffer&& buffer)
	{
		m_type      = buffer.m_type;
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
			init(buffer.m_type, buffer.m_idx_count, reinterpret_cast<const uint16_t*>(buffer.m_data), keep_cpu_data);
		}
		else
		{
			init(buffer.m_type, buffer.m_idx_count, reinterpret_cast<const uint32_t*>(buffer.m_data), keep_cpu_data);
		}
		return *this;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& buffer)
	{
		if (this == &buffer)
			return *this;

		m_type      = buffer.m_type;
		m_format    = buffer.m_format;
		m_idx_count = buffer.m_idx_count;
		m_data      = buffer.m_data;
		m_buffer    = std::move(buffer.m_buffer);

		buffer.m_idx_count = 0;
		buffer.m_data      = nullptr;
		return *this;
	}

	IndexBuffer& IndexBuffer::init(RHIBufferType type, size_t size, const uint16_t* data, bool keep_cpu_data)
	{
		allocate_data(type, RHIIndexFormat::UInt16, sizeof(uint16_t) * size);
		std::memcpy(m_data, data, sizeof(uint16_t) * size);
		return init(keep_cpu_data);
	}

	IndexBuffer& IndexBuffer::init(RHIBufferType type, size_t size, const uint32_t* data, bool keep_cpu_data)
	{
		allocate_data(type, RHIIndexFormat::UInt32, sizeof(uint32_t) * size);
		std::memcpy(m_data, data, sizeof(uint32_t) * size);
		return init(keep_cpu_data);
		return *this;
	}

	IndexBuffer& IndexBuffer::init(bool keep_cpu_data)
	{
		if (m_idx_count > 0 && m_buffer == nullptr)
		{
			auto create_buffer = [this, keep_cpu_data]() {
				m_buffer = rhi->create_index_buffer(size(), m_data, m_format, m_type);

				if (!keep_cpu_data && m_data && !Settings::Rendering::force_keep_cpu_resources)
				{
					release_memory(m_data);
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

	byte* IndexBuffer::allocate_data(RHIBufferType type, RHIIndexFormat format, size_t count)
	{
		release();
		m_type      = type;
		m_format    = format;
		m_idx_count = count;

		m_data = allocate_memory(size(), alignof(uint32_t));
		return m_data;
	}

	IndexBuffer& IndexBuffer::release()
	{
		if (m_data)
		{
			release_memory(m_data);
			m_data = nullptr;
		}

		m_idx_count = 0;
		m_buffer    = nullptr;
		return *this;
	}

	IndexBuffer& IndexBuffer::rhi_bind(size_t offset)
	{
		if (m_buffer)
		{
			m_buffer->bind(offset);
		}
		return *this;
	}

	bool IndexBuffer::serialize(Archive& ar)
	{
		bool has_data = m_data != nullptr;
		ar.serialize(m_idx_count, m_type, m_format, has_data);

		if (has_data)
		{
			if (ar.is_reading())
			{
				allocate_data(m_type, m_format, m_idx_count);
			}

			ar.write_data(m_data, size());
		}
		return ar;
	}
}// namespace Engine
