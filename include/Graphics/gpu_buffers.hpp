#pragma once
#include <Core/archive.hpp>
#include <Core/engine_types.hpp>
#include <Core/render_resource_ptr.hpp>
#include <Graphics/rhi.hpp>
#include <initializer_list>

namespace Engine
{
	// VERTEX BUFFER IMPLEMENTATION

	class ENGINE_EXPORT VertexBufferBase
	{
	private:
		RenderResourcePtr<RHI_VertexBuffer> m_buffer;
		byte* m_data         = nullptr;
		uint32_t m_vtx_count = 0;
		uint16_t m_stride    = 0;
		RHIBufferType m_type = RHIBufferType::Static;

	public:
		VertexBufferBase();
		// clang-format off
		VertexBufferBase(RHIBufferType type, uint16_t stride, size_t count, const void* data = nullptr, bool keep_cpu_data = false);
		VertexBufferBase(const VertexBufferBase& buffer);
		VertexBufferBase(VertexBufferBase&& buffer);
		VertexBufferBase& operator=(const VertexBufferBase& buffer);
		VertexBufferBase& operator=(VertexBufferBase&& buffer);

		VertexBufferBase& init(RHIBufferType type, size_t stride, size_t count, const void* data = nullptr, bool keep_cpu_data = false);
		// clang-format on

		VertexBufferBase& init(bool keep_cpu_data = false);
		byte* allocate_data(RHIBufferType type, uint16_t stride, size_t count);
		VertexBufferBase& release();

		VertexBufferBase& rhi_bind(byte stream_index, size_t offset = 0);
		VertexBufferBase& rhi_update(size_t size, size_t offset = 0);

		bool serialize(Archive& ar);

		inline RHI_VertexBuffer* rhi_vertex_buffer() { return m_buffer; }
		inline byte* data() { return m_data; }
		inline const byte* data() const { return m_data; }
		inline RHIBufferType type() const { return m_type; }
		inline size_t size() const { return static_cast<size_t>(m_vtx_count) * static_cast<size_t>(m_stride); }
		inline size_t stride() const { return static_cast<size_t>(m_stride); }
		inline size_t vertices() const { return static_cast<size_t>(m_vtx_count); }
	};

	template<typename T>
	class VertexBuffer : public VertexBufferBase
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(const std::initializer_list<T>& list, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false)
			: VertexBufferBase(type, sizeof(T), list.size(), reinterpret_cast<const byte*>(list.begin()), keep_cpu_data)
		{}

		VertexBuffer(const T* data, size_t count, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false)
			: VertexBufferBase(type, sizeof(T), count, reinterpret_cast<const byte*>(data), keep_cpu_data)
		{}

		VertexBuffer(const T* begin, const T* end, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false)
			: VertexBufferBase(type, sizeof(T), end - begin, reinterpret_cast<const byte*>(begin), keep_cpu_data)
		{}

		inline VertexBuffer& init(RHIBufferType type, size_t count, const T* data = nullptr, bool keep_cpu_data = false)
		{
			VertexBufferBase::init(type, sizeof(T), count, reinterpret_cast<const byte*>(data), keep_cpu_data);
			return *this;
		}

		inline VertexBuffer& init(bool keep_cpu_data = false)
		{
			VertexBufferBase::init(keep_cpu_data);
			return *this;
		}

		inline T* allocate_data(RHIBufferType type, size_t size)
		{
			return reinterpret_cast<T*>(VertexBufferBase::allocate_data(type, sizeof(T), size));
		}

		inline VertexBuffer& rhi_bind(byte stream_index, size_t offset = 0)
		{
			VertexBufferBase::rhi_bind(stream_index, offset);
			return *this;
		}

		inline T* data() { return reinterpret_cast<T*>(VertexBufferBase::data()); }
		inline const T* data() const { return reinterpret_cast<T*>(VertexBufferBase::data()); }
	};

	// clang-format off
	class PositionVertexBuffer  : public VertexBuffer<Vector3f>  { using VertexBuffer::VertexBuffer; };
	class TexCoordVertexBuffer  : public VertexBuffer<Vector2f>	 { using VertexBuffer::VertexBuffer; };
	class ColorVertexBuffer     : public VertexBuffer<ByteColor> { using VertexBuffer::VertexBuffer; };
	class NormalVertexBuffer    : public VertexBuffer<Vector3f>  { using VertexBuffer::VertexBuffer; };
	class TangentVertexBuffer   : public VertexBuffer<Vector3f>  { using VertexBuffer::VertexBuffer; };
	class BitangentVertexBuffer : public VertexBuffer<Vector3f>  { using VertexBuffer::VertexBuffer; };
	// clang-format on


	// INDEX BUFFER IMPLEMENTATION

	class ENGINE_EXPORT IndexBuffer
	{
	private:
		RenderResourcePtr<RHI_IndexBuffer> m_buffer;
		byte* m_data            = nullptr;
		uint32_t m_idx_count    = 0;
		RHIBufferType m_type    = RHIBufferType::Static;
		RHIIndexFormat m_format = RHIIndexFormat::UInt16;

	public:
		IndexBuffer();
		// clang-format off
		IndexBuffer(RHIBufferType type, size_t count, const uint16_t* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer(RHIBufferType type, size_t count, const uint32_t* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer(const std::initializer_list<uint16_t>& list, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false);
		IndexBuffer(const std::initializer_list<uint32_t>& list, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false);
		IndexBuffer(const uint16_t* start, const uint16_t* end, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false);
		IndexBuffer(const uint32_t* start, const uint32_t* end, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false);
		IndexBuffer(const IndexBuffer& buffer);
		IndexBuffer(IndexBuffer&& buffer);
		IndexBuffer& operator=(const IndexBuffer& buffer);
		IndexBuffer& operator=(IndexBuffer&& buffer);
		// clang-format on

		IndexBuffer& init(RHIBufferType type, size_t count, const uint16_t* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer& init(RHIBufferType type, size_t count, const uint32_t* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer& init(bool keep_cpu_data = false);
		byte* allocate_data(RHIBufferType type, RHIIndexFormat format, size_t count);
		IndexBuffer& release();
		IndexBuffer& rhi_bind(size_t offset = 0);

		bool serialize(Archive& ar);

		inline RHI_IndexBuffer* rhi_index_buffer() { return m_buffer; }
		inline byte* data() { return m_data; }
		inline const byte* data() const { return m_data; }
		inline RHIBufferType type() const { return m_type; }
		inline RHIIndexFormat format() const { return m_format; }
		inline size_t stride() const { return m_format == RHIIndexFormat::UInt16 ? 2 : 4; }
		inline size_t size() const { return static_cast<size_t>(m_idx_count) * stride(); }
		inline size_t indices_count() const { return static_cast<size_t>(m_idx_count); }
	};

	template<typename T>
	class TypedIndexBuffer : public IndexBuffer
	{
	public:
		TypedIndexBuffer() = default;

		TypedIndexBuffer(RHIBufferType type, size_t count, const T* data = nullptr, bool keep_cpu_data = false)
			: IndexBuffer(type, size, data, keep_cpu_data)
		{}

		TypedIndexBuffer(const std::initializer_list<T>& list, RHIBufferType type = RHIBufferType::Static,
						 bool keep_cpu_data = false)
			: IndexBuffer(list, type, keep_cpu_data)
		{}

		TypedIndexBuffer(const T* start, const T* end, RHIBufferType type = RHIBufferType::Static, bool keep_cpu_data = false)
			: IndexBuffer(start, end, type, keep_cpu_data)
		{}

		TypedIndexBuffer& init(RHIBufferType type, size_t count, const T* data = nullptr, bool keep_cpu_data = false)
		{
			IndexBuffer::init(type, count, data, keep_cpu_data);
			return *this;
		}

		TypedIndexBuffer& init(bool keep_cpu_data = false)
		{
			IndexBuffer::init(keep_cpu_data);
			return *this;
		}

		T* allocate_data(RHIBufferType type, size_t count)
		{
			constexpr RHIIndexFormat format = sizeof(T) == 2 ? RHIIndexFormat::UInt16 : RHIIndexFormat::UInt32;
			return reinterpret_cast<T*>(IndexBuffer::allocate_data(type, format, count));
		}

		T* data() { return reinterpret_cast<T*>(IndexBuffer::data()); }
		const T* data() const { return reinterpret_cast<T*>(IndexBuffer::data()); }
	};

	// clang-format off
	class IndexBuffer16 : public TypedIndexBuffer<uint16_t> { using TypedIndexBuffer::TypedIndexBuffer; };
	class IndexBuffer32 : public TypedIndexBuffer<uint32_t> { using TypedIndexBuffer::TypedIndexBuffer; };
	// clang-format on
}// namespace Engine
