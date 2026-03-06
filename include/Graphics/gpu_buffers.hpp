#pragma once
#include <Core/engine_types.hpp>
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>
#include <RHI/resource_ptr.hpp>
#include <initializer_list>

namespace Engine
{
	// VERTEX BUFFER IMPLEMENTATION
	class RHIBuffer;
	class Archive;

	class ENGINE_EXPORT VertexBufferBase
	{
	private:
		RHIResourcePtr<RHIBuffer> m_buffer;
		u8* m_data                   = nullptr;
		u32 m_vtx_count              = 0;
		u16 m_stride                 = 0;
		RHIBufferCreateFlags m_flags = RHIBufferCreateFlags::Static;

	public:
		static const VertexBufferBase* static_null();

	public:
		VertexBufferBase();
		~VertexBufferBase();

		// clang-format off
		VertexBufferBase(RHIBufferCreateFlags type, u16 stride, usize count, const void* data = nullptr, bool keep_cpu_data = false);
		VertexBufferBase(const VertexBufferBase& buffer);
		VertexBufferBase(VertexBufferBase&& buffer);
		VertexBufferBase& operator=(const VertexBufferBase& buffer);
		VertexBufferBase& operator=(VertexBufferBase&& buffer);

		VertexBufferBase& init(RHIBufferCreateFlags type, usize stride, usize count, const void* data = nullptr, bool keep_cpu_data = false);
		// clang-format on

		VertexBufferBase& init(bool keep_cpu_data = false);
		u8* allocate_data(RHIBufferCreateFlags type, u16 stride, usize count);
		VertexBufferBase& release();
		VertexBufferBase& grow(u32 factor = 2);

		VertexBufferBase& rhi_update(class RHIContext* ctx, usize size, usize offset = 0);

		bool serialize(Archive& ar);

		inline RHIBuffer* rhi_buffer() const { return m_buffer; }
		inline u8* data() { return m_data; }
		inline const u8* data() const { return m_data; }
		inline RHIBufferCreateFlags flags() const { return m_flags; }
		inline usize size() const { return static_cast<usize>(m_vtx_count) * static_cast<usize>(m_stride); }
		inline usize stride() const { return static_cast<usize>(m_stride); }
		inline usize vertices() const { return static_cast<usize>(m_vtx_count); }

		friend class NullVertexBuffer;
	};

	template<typename T>
	class VertexBuffer : public VertexBufferBase
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(const std::initializer_list<T>& list, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static,
		             bool keep_cpu_data = false)
		    : VertexBufferBase(type, sizeof(T), list.size(), reinterpret_cast<const u8*>(list.begin()), keep_cpu_data)
		{}

		VertexBuffer(RHIBufferCreateFlags type, usize count, const T* data = nullptr, bool keep_cpu_data = false)
		    : VertexBufferBase(type, sizeof(T), count, reinterpret_cast<const u8*>(data), keep_cpu_data)
		{}

		VertexBuffer(const T* begin, const T* end, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static,
		             bool keep_cpu_data = false)
		    : VertexBufferBase(type, sizeof(T), end - begin, reinterpret_cast<const u8*>(begin), keep_cpu_data)
		{}

		inline VertexBuffer& init(RHIBufferCreateFlags type, usize count, const T* data = nullptr, bool keep_cpu_data = false)
		{
			VertexBufferBase::init(type, sizeof(T), count, reinterpret_cast<const u8*>(data), keep_cpu_data);
			return *this;
		}

		inline VertexBuffer& init(bool keep_cpu_data = false)
		{
			VertexBufferBase::init(keep_cpu_data);
			return *this;
		}

		inline T* allocate_data(RHIBufferCreateFlags type, usize size)
		{
			return reinterpret_cast<T*>(VertexBufferBase::allocate_data(type, sizeof(T), size));
		}

		inline T* data() { return reinterpret_cast<T*>(VertexBufferBase::data()); }
		inline const T* data() const { return reinterpret_cast<T*>(VertexBufferBase::data()); }
	};

	// clang-format off
	class PositionVertexBuffer  : public VertexBuffer<Vector3f>  { using VertexBuffer::VertexBuffer; };
	class TexCoordVertexBuffer  : public VertexBuffer<Vector2f>	 { using VertexBuffer::VertexBuffer; };
	class ColorVertexBuffer     : public VertexBuffer<Color> { using VertexBuffer::VertexBuffer; };
	class NormalVertexBuffer    : public VertexBuffer<Vector3f>  { using VertexBuffer::VertexBuffer; };
	class TangentVertexBuffer   : public VertexBuffer<Vector4f>  { using VertexBuffer::VertexBuffer; };
	class BlendWeightVertexBuffer  : public VertexBuffer<Vector4f>  { using VertexBuffer::VertexBuffer; };
	class BlendIndicesVertexBuffer : public VertexBuffer<Vector4u16>  { using VertexBuffer::VertexBuffer; };
	// clang-format on


	// INDEX BUFFER IMPLEMENTATION

	class ENGINE_EXPORT IndexBuffer
	{
	private:
		RHIResourcePtr<RHIBuffer> m_buffer;
		u8* m_data                   = nullptr;
		u32 m_idx_count              = 0;
		RHIBufferCreateFlags m_flags = RHIBufferCreateFlags::Static;
		RHIIndexFormat m_format      = RHIIndexFormat::UInt16;

	public:
		IndexBuffer();
		~IndexBuffer();
		// clang-format off
		IndexBuffer(RHIBufferCreateFlags type, usize count, const u16* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer(RHIBufferCreateFlags type, usize count, const u32* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer(const std::initializer_list<u16>& list, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static, bool keep_cpu_data = false);
		IndexBuffer(const std::initializer_list<u32>& list, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static, bool keep_cpu_data = false);
		IndexBuffer(const u16* start, const u16* end, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static, bool keep_cpu_data = false);
		IndexBuffer(const u32* start, const u32* end, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static, bool keep_cpu_data = false);
		IndexBuffer(const IndexBuffer& buffer);
		IndexBuffer(IndexBuffer&& buffer);
		IndexBuffer& operator=(const IndexBuffer& buffer);
		IndexBuffer& operator=(IndexBuffer&& buffer);
		// clang-format on

		IndexBuffer& init(RHIBufferCreateFlags type, usize count, const u16* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer& init(RHIBufferCreateFlags type, usize count, const u32* data = nullptr, bool keep_cpu_data = false);
		IndexBuffer& init(bool keep_cpu_data = false);
		u8* allocate_data(RHIBufferCreateFlags type, RHIIndexFormat format, usize count);
		IndexBuffer& release();

		bool serialize(Archive& ar);

		inline RHIBuffer* rhi_buffer() const { return m_buffer; }
		inline u8* data() { return m_data; }
		inline const u8* data() const { return m_data; }
		inline RHIBufferCreateFlags flags() const { return m_flags; }
		inline RHIIndexFormat format() const { return m_format; }
		inline usize stride() const { return m_format == RHIIndexFormat::UInt16 ? 2 : 4; }
		inline usize size() const { return static_cast<usize>(m_idx_count) * stride(); }
		inline usize indices_count() const { return static_cast<usize>(m_idx_count); }
	};

	template<typename T>
	class TypedIndexBuffer : public IndexBuffer
	{
	public:
		TypedIndexBuffer() = default;

		TypedIndexBuffer(RHIBufferCreateFlags type, usize count, const T* data = nullptr, bool keep_cpu_data = false)
		    : IndexBuffer(type, size, data, keep_cpu_data)
		{}

		TypedIndexBuffer(const std::initializer_list<T>& list, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static,
		                 bool keep_cpu_data = false)
		    : IndexBuffer(list, type, keep_cpu_data)
		{}

		TypedIndexBuffer(const T* start, const T* end, RHIBufferCreateFlags type = RHIBufferCreateFlags::Static,
		                 bool keep_cpu_data = false)
		    : IndexBuffer(start, end, type, keep_cpu_data)
		{}

		TypedIndexBuffer& init(RHIBufferCreateFlags type, usize count, const T* data = nullptr, bool keep_cpu_data = false)
		{
			IndexBuffer::init(type, count, data, keep_cpu_data);
			return *this;
		}

		TypedIndexBuffer& init(bool keep_cpu_data = false)
		{
			IndexBuffer::init(keep_cpu_data);
			return *this;
		}

		T* allocate_data(RHIBufferCreateFlags type, usize count)
		{
			constexpr RHIIndexFormat format = sizeof(T) == 2 ? RHIIndexFormat::UInt16 : RHIIndexFormat::UInt32;
			return reinterpret_cast<T*>(IndexBuffer::allocate_data(type, format, count));
		}

		T* data() { return reinterpret_cast<T*>(IndexBuffer::data()); }
		const T* data() const { return reinterpret_cast<T*>(IndexBuffer::data()); }
	};

	// clang-format off
	class IndexBuffer16 : public TypedIndexBuffer<u16> { using TypedIndexBuffer::TypedIndexBuffer; };
	class IndexBuffer32 : public TypedIndexBuffer<u32> { using TypedIndexBuffer::TypedIndexBuffer; };
	// clang-format on
}// namespace Engine
