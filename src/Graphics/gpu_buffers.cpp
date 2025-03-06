#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/settings.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
	trinex_implement_engine_class_default_init(GPUBuffer, 0);
	trinex_implement_engine_class_default_init(SSBO, 0);

	bool GPUBuffer::force_keep_cpu_data()
	{
		return Settings::Rendering::force_keep_cpu_resources;
	}

	GPUBuffer& GPUBuffer::rhi_update(size_t offset, size_t size, const byte* data)
	{
		if (m_rhi_object)
		{
			rhi_object<RHI_Buffer>()->update(offset, size, data);
		}
		return *this;
	}

	size_t GPUBuffer::size() const
	{
		return m_size;
	}

	byte* GPUBuffer::data()
	{
		return nullptr;
	}

	const byte* GPUBuffer::data() const
	{
		return nullptr;
	}

	RHIBufferType GPUBuffer::buffer_type() const
	{
		return RHIBufferType::Static;
	}

	VertexBuffer& VertexBuffer::rhi_init()
	{
		size_t s                 = size();
		RHI_VertexBuffer* buffer = nullptr;

		if (s > 0)
		{
			buffer = rhi->create_vertex_buffer(s, data(), buffer_type());
		}

		m_rhi_object.reset(buffer);
		return *this;
	}

	VertexBuffer& VertexBuffer::rhi_init(size_t size, const byte* data)
	{
		m_size = size;
		m_rhi_object.reset(rhi->create_vertex_buffer(size, data, buffer_type()));
		return *this;
	}

	VertexBuffer& VertexBuffer::rhi_bind(byte stream_index, size_t offset)
	{
		if (m_rhi_object)
		{
			rhi_object<RHI_VertexBuffer>()->bind(stream_index, stride(), offset);
		}

		return *this;
	}

	size_t VertexBuffer::vertex_count() const
	{
		return size() / stride();
	}

	template<typename Value, typename T>
	static Value* find_vertex_address(T* buffer, size_t index)
	{
		if (Value* ptr = buffer->data())
		{
			size_t v_stride = buffer->stride();
			size_t v_count  = buffer->size() / v_stride;

			if (index < v_count)
			{
				return ptr + index * v_stride;
			}
		}
		return nullptr;
	}

	const byte* VertexBuffer::vertex_address(size_t index) const
	{
		return find_vertex_address<const byte>(this, index);
	}

	byte* VertexBuffer::vertex_address(size_t index)
	{
		return find_vertex_address<byte>(this, index);
	}

	trinex_implement_engine_class_default_init(VertexBuffer, 0);
	trinex_implement_engine_class_default_init(PositionVertexBuffer, 0);
	trinex_implement_engine_class_default_init(TexCoordVertexBuffer, 0);
	trinex_implement_engine_class_default_init(ColorVertexBuffer, 0);
	trinex_implement_engine_class_default_init(NormalVertexBuffer, 0);
	trinex_implement_engine_class_default_init(TangentVertexBuffer, 0);
	trinex_implement_engine_class_default_init(BitangentVertexBuffer, 0);
	trinex_implement_engine_class_default_init(PositionDynamicVertexBuffer, 0);
	trinex_implement_engine_class_default_init(TexCoordDynamicVertexBuffer, 0);
	trinex_implement_engine_class_default_init(ColorDynamicVertexBuffer, 0);
	trinex_implement_engine_class_default_init(NormalDynamicVertexBuffer, 0);
	trinex_implement_engine_class_default_init(TangentDynamicVertexBuffer, 0);
	trinex_implement_engine_class_default_init(BitangentDynamicVertexBuffer, 0);

	//////////////////////////// INDEX BUFFER ////////////////////////////

	trinex_implement_engine_class_default_init(IndexBuffer, 0);
	trinex_implement_engine_class_default_init(UInt32IndexBuffer, 0);
	trinex_implement_engine_class_default_init(UInt16IndexBuffer, 0);
	trinex_implement_engine_class_default_init(UInt32DynamicIndexBuffer, 0);
	trinex_implement_engine_class_default_init(UInt16DynamicIndexBuffer, 0);

	IndexBuffer& IndexBuffer::rhi_init()
	{
		size_t s                = size();
		RHI_IndexBuffer* buffer = nullptr;

		if (s > 0)
		{
			buffer = rhi->create_index_buffer(s, data(), index_type(), buffer_type());
		}

		m_rhi_object.reset(buffer);
		return *this;
	}

	IndexBuffer& IndexBuffer::rhi_init(size_t size, const byte* data)
	{
		m_size = size;
		m_rhi_object.reset(rhi->create_index_buffer(size, data, index_type(), buffer_type()));
		return *this;
	}

	IndexBuffer& IndexBuffer::rhi_bind(size_t offset)
	{
		if (m_rhi_object)
		{
			rhi_object<RHI_IndexBuffer>()->bind(offset);
		}
		return *this;
	}

	size_t IndexBuffer::index_count() const
	{
		return size() / index_size();
	}

	template<typename Value, typename T>
	static Value* find_index_address(T* buffer, size_t index)
	{
		if (Value* ptr = buffer->data())
		{
			size_t i_size  = buffer->index_size();
			size_t i_count = buffer->size() / i_size;

			if (index < i_count)
			{
				return ptr + index * i_size;
			}
		}
		return nullptr;
	}

	const byte* IndexBuffer::index_address(size_t index) const
	{
		return find_index_address<const byte>(this, index);
	}

	byte* IndexBuffer::index_address(size_t index)
	{
		return find_index_address<byte>(this, index);
	}

	// UNIFORM BUFFERS
	trinex_implement_engine_class_default_init(UniformBuffer, 0);
	trinex_implement_engine_class_default_init(UntypedUniformBuffer, 0);

	UniformBuffer& UniformBuffer::rhi_init()
	{
		trinex_check(m_size != 0, "Size of uniform buffer can't be 0");
		return rhi_init(size(), data());
	}

	UniformBuffer& UniformBuffer::rhi_init(size_t size, const byte* data)
	{
		m_rhi_object.reset(rhi->create_uniform_buffer(size, data, buffer_type()));
		return *this;
	}

	RHIBufferType UniformBuffer::buffer_type() const
	{
		return RHIBufferType::Dynamic;
	}

	SSBO& SSBO::rhi_init()
	{
		m_rhi_object.reset(rhi->create_ssbo(init_size, init_data, RHIBufferType::Static));
		return *this;
	}

	SSBO& SSBO::rhi_bind(BindLocation location)
	{
		if (m_rhi_object)
		{
			rhi_object<RHI_SSBO>()->bind(location);
		}

		return *this;
	}
}// namespace Engine
