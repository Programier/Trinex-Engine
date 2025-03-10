#pragma once
#include <Core/archive.hpp>
#include <Core/engine_types.hpp>
#include <Core/render_resource.hpp>

namespace Engine
{
	class ENGINE_EXPORT GPUBuffer : public RenderResource
	{
		trinex_declare_class(GPUBuffer, RenderResource);

	protected:
		size_t m_size = 0;
		static bool force_keep_cpu_data();

		void execute_discard();

	public:
		GPUBuffer& rhi_update(size_t offset, size_t size, const byte* data);
		size_t size() const;

		virtual GPUBuffer& discard();
		virtual byte* data();
		virtual const byte* data() const;
		virtual RHIBufferType buffer_type() const;
		virtual RHI_Buffer* rhi_buffer() const = 0;
	};

	template<typename Type, typename Super>
	class TypedGPUBuffer : public Super
	{
		struct Buffer {
			bool m_with_cpu_access;
			Vector<Type> m_buffer;
		};

		Buffer* m_buffer = nullptr;

	public:
		using ElementType = Type;

	public:
		using Super::init_render_resources;

		TypedGPUBuffer& init(size_t buffer_size, const ElementType* buffer_data = nullptr, bool with_cpu_access = false)
		{
			auto& buffer = allocate_data(with_cpu_access);

			if (buffer_data)
			{
				buffer.assign(buffer_data, buffer_data + buffer_size);
			}
			else
			{
				buffer.resize(buffer_size);
			}
			return *this;
		}

		TypedGPUBuffer& init(const Vector<ElementType>& buffer, bool with_cpu_access = false)
		{
			return init(buffer.size(), buffer.data(), with_cpu_access);
		}

		Vector<Type>& allocate_data(bool with_cpu_access)
		{
			if (m_buffer)
			{
				delete m_buffer;
			}

			m_buffer                    = new Buffer();
			m_buffer->m_with_cpu_access = with_cpu_access;
			return m_buffer->m_buffer;
		}

		bool with_cpu_access() const { return m_buffer ? m_buffer->m_with_cpu_access : false; }

		TypedGPUBuffer& init_render_resources() override final
		{
			Super::init_render_resources();
			return *this;
		}

		byte* data() override final
		{
			if (m_buffer)
			{
				return reinterpret_cast<byte*>(m_buffer->m_buffer.data());
			}
			return nullptr;
		}

		const byte* data() const override final
		{
			if (m_buffer)
			{
				return reinterpret_cast<const byte*>(m_buffer->m_buffer.data());
			}
			return nullptr;
		}

		Vector<Type>* buffer() { return m_buffer ? &m_buffer->m_buffer : nullptr; }

		const Vector<Type>* buffer() const { return m_buffer ? &m_buffer->m_buffer : nullptr; }

		TypedGPUBuffer& discard() override
		{
			if (m_buffer && !m_buffer->m_with_cpu_access && !GPUBuffer::force_keep_cpu_data())
			{
				delete m_buffer;
				m_buffer = nullptr;
			}
			return *this;
		}

		bool serialize(Archive& ar) override final { return serialize(ar, false); }

		virtual bool serialize(Archive& ar, bool allow_cpu_access)
		{
			if (!Super::serialize(ar))
				return false;

			if (ar.is_reading())
			{
				bool has_data      = false;
				size_t buffer_size = 0;

				ar.serialize(has_data);
				ar.serialize(buffer_size);

				auto& buffer = allocate_data(allow_cpu_access);
				buffer.resize(buffer_size / sizeof(Type));

				if (has_data)
				{
					byte* ptr = reinterpret_cast<byte*>(buffer.data());
					ar.read_data(ptr, buffer_size);
				}
			}
			else
			{
				bool has_data      = m_buffer != nullptr;
				size_t buffer_size = has_data ? m_buffer->m_buffer.size() * sizeof(Type) : Super::size();

				ar.serialize(has_data);
				ar.serialize(buffer_size);

				if (has_data)
				{
					byte* ptr = reinterpret_cast<byte*>(m_buffer->m_buffer.data());
					ar.write_data(ptr, buffer_size);
				}
			}
			return ar;
		}

		~TypedGPUBuffer()
		{
			if (m_buffer)
			{
				delete m_buffer;
				m_buffer = nullptr;
			}
		}
	};

	///////////////// VERTEX BUFFERS /////////////////

	class ENGINE_EXPORT VertexBuffer : public GPUBuffer
	{
		trinex_declare_class(VertexBuffer, GPUBuffer);

		RenderResourcePtr<RHI_VertexBuffer> m_buffer;

	public:
		VertexBuffer& init_render_resources() override;
		VertexBuffer& release_render_resources() override;
		VertexBuffer& rhi_bind(byte stream_index, size_t offset = 0);
		size_t vertex_count() const;
		const byte* vertex_address(size_t index) const;
		byte* vertex_address(size_t index);
		RHI_Buffer* rhi_buffer() const override;

		virtual size_t stride() const = 0;

		inline RHI_VertexBuffer* rhi_vertex_buffer() const { return m_buffer; }
	};

	template<typename Type>
	class TypedVertexBuffer : public TypedGPUBuffer<Type, VertexBuffer>
	{
	public:
		size_t stride() const override { return sizeof(Type); }
	};

	template<typename Type>
	class TypedDynamicVertexBuffer : public TypedVertexBuffer<Type>
	{
	public:
		RHIBufferType buffer_type() const override { return RHIBufferType::Dynamic; }
	};

	class ENGINE_EXPORT PositionVertexBuffer : public TypedVertexBuffer<Vector3f>
	{
		trinex_declare_class(PositionVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT TexCoordVertexBuffer : public TypedVertexBuffer<Vector2f>
	{
		trinex_declare_class(TexCoordVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT ColorVertexBuffer : public TypedVertexBuffer<ByteColor>
	{
		trinex_declare_class(ColorVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT NormalVertexBuffer : public TypedVertexBuffer<Vector3f>
	{
		trinex_declare_class(NormalVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT TangentVertexBuffer : public TypedVertexBuffer<Vector3f>
	{
		trinex_declare_class(TangentVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT BitangentVertexBuffer : public TypedVertexBuffer<Vector3f>
	{
		trinex_declare_class(BitangentVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT PositionDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3f>
	{
		trinex_declare_class(PositionDynamicVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT TexCoordDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector2f>
	{
		trinex_declare_class(TexCoordDynamicVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT ColorDynamicVertexBuffer : public TypedDynamicVertexBuffer<ByteColor>
	{
		trinex_declare_class(ColorDynamicVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT NormalDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3f>
	{
		trinex_declare_class(NormalDynamicVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT TangentDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3f>
	{
		trinex_declare_class(TangentDynamicVertexBuffer, VertexBuffer);
	};

	class ENGINE_EXPORT BitangentDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3f>
	{
		trinex_declare_class(BitangentDynamicVertexBuffer, VertexBuffer);
	};

	///////////////// INDEX BUFFER /////////////////

	class ENGINE_EXPORT IndexBuffer : public GPUBuffer
	{
		trinex_declare_class(IndexBuffer, GPUBuffer);

		RenderResourcePtr<RHI_IndexBuffer> m_buffer;

	public:
		IndexBuffer& init_render_resources() override;
		IndexBuffer& release_render_resources() override;
		IndexBuffer& rhi_bind(size_t offset = 0);

		size_t index_count() const;
		const byte* index_address(size_t index) const;
		byte* index_address(size_t index);
		RHI_Buffer* rhi_buffer() const override;

		virtual IndexBufferFormat index_type() const = 0;
		virtual size_t index_size() const            = 0;

		inline RHI_IndexBuffer* rhi_index_buffer() const { return m_buffer; }
	};

	template<typename Type, IndexBufferFormat format>
	class TypedIndexBuffer : public TypedGPUBuffer<Type, IndexBuffer>
	{
	public:
		IndexBufferFormat index_type() const override { return format; }

		size_t index_size() const override { return sizeof(Type); }
	};

	template<typename Type, IndexBufferFormat format>
	class TypedDynamicIndexBuffer : public TypedIndexBuffer<Type, format>
	{
	public:
		RHIBufferType buffer_type() const override { return RHIBufferType::Dynamic; }
	};

	class ENGINE_EXPORT UInt32IndexBuffer : public TypedIndexBuffer<uint32_t, IndexBufferFormat::UInt32>
	{
		trinex_declare_class(UInt32IndexBuffer, IndexBuffer);
	};

	class ENGINE_EXPORT UInt16IndexBuffer : public TypedIndexBuffer<uint32_t, IndexBufferFormat::UInt16>
	{
		trinex_declare_class(UInt16IndexBuffer, IndexBuffer);
	};

	class ENGINE_EXPORT UInt32DynamicIndexBuffer : public TypedDynamicIndexBuffer<uint32_t, IndexBufferFormat::UInt32>
	{
		trinex_declare_class(UInt32DynamicIndexBuffer, IndexBuffer);
	};

	class ENGINE_EXPORT UInt16DynamicIndexBuffer : public TypedDynamicIndexBuffer<uint32_t, IndexBufferFormat::UInt16>
	{
		trinex_declare_class(UInt16DynamicIndexBuffer, IndexBuffer);
	};

	// UNIFORM BUFFER

	class ENGINE_EXPORT UniformBuffer : public GPUBuffer
	{
		trinex_declare_class(UniformBuffer, GPUBuffer);

		RenderResourcePtr<RHI_UniformBuffer> m_buffer;

	public:
		UniformBuffer& init_render_resources() override;
		UniformBuffer& release_render_resources() override;
		RHIBufferType buffer_type() const override;
		RHI_Buffer* rhi_buffer() const override;

		inline RHI_UniformBuffer* rhi_uniform_buffer() const { return m_buffer; }
	};

	template<typename StorageType>
	class ENGINE_EXPORT StructuredUniformBuffer : public UniformBuffer
	{
	public:
		StorageType storage;

		StructuredUniformBuffer& init_render_resources() override
		{
			m_size = sizeof(StorageType);
			UniformBuffer::init_render_resources();
			return *this;
		}

		byte* data() override { return reinterpret_cast<byte*>(&storage); }
		const byte* data() const override { return reinterpret_cast<const byte*>(&storage); }
		using UniformBuffer::rhi_update;

		StructuredUniformBuffer& rhi_update(size_t offset, size_t size)
		{
			offset = glm::clamp(offset, static_cast<size_t>(0), static_cast<size_t>(sizeof(StorageType)));
			size   = glm::min(size, static_cast<size_t>(sizeof(StorageType)) - offset);
			UniformBuffer::rhi_update(offset, size, data() + offset);
			return *this;
		}
	};

	class ENGINE_EXPORT UntypedUniformBuffer : public TypedGPUBuffer<byte, UniformBuffer>
	{
		trinex_declare_class(UntypedUniformBuffer, UniformBuffer);
	};

	class ENGINE_EXPORT SSBO : public GPUBuffer
	{
		trinex_declare_class(SSBO, GPUBuffer);

	public:
		size_t init_size      = 0;
		const byte* init_data = nullptr;

		SSBO& init_render_resources() override;
		SSBO& rhi_bind(BindLocation location);
	};
}// namespace Engine
