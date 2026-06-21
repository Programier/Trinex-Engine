#pragma once
#include <Core/etl/vector.hpp>
#include <Core/math/box.hpp>
#include <Core/math/matrix.hpp>
#include <Engine/enviroment.hpp>
#include <RHI/types.hpp>
#include <tlfs/tlsf.hpp>

namespace Trinex
{
	class PrimitiveComponent;
	class LightComponent;
	class PostProcessComponent;
	class Material;
	class MaterialInterface;
	struct Frustum;
	class RHIContext;
	class RHIBuffer;

	class ENGINE_EXPORT RenderScene final
	{
	public:
		struct Geometry {
			struct Buffer {
				RHIDescriptor buffer;
				u32 offset;
				u32 stride;
			};

			using VertexStream = Vector3f;

			struct SurfaceStream {
				Vector2f16 uv0;
				Vector2f16 uv1;

				u32 normal;
				u32 tangent;
			};

			struct AnimationStream {
				u8 indices[4];
				u8 weights[4];
			};

			using IndexStream = u32;

			Buffer vertex_stream    = {};
			Buffer surface_stream   = {};
			Buffer animation_stream = {};
			Buffer index_stream     = {};
			Box3f aabb              = {};
		};

		struct Primitive {
			struct Flags {
				enum Enum : u32
				{
					IsLast   = BIT(0),
					UserMask = 0,
				};

				trinex_bitfield_enum_struct(Flags, u32);
			};
			using enum Flags::Enum;

			MaterialInterface* material;
			u32 first_vertex   = 0;
			u32 first_index    = 0;
			u32 vertices_count = 0;
			u32 transform      = 0;
			u32 data           = 0;
			u32 chunk          = 0;
			u32 geometry       = 0;
			Flags flags        = 0;
		};

		struct Chunk {
			Material* material = nullptr;
			u32 count          = 0;
			u32 capacity       = 0;
			u32 address        = 0;
		};

	private:
		struct Command {
			u32 begin;
			u32 end;
			inline bool operator<(const Command& other) const { return begin < other.begin; }
		};

		Vector<u8> m_cpu_heap;
		RHIBuffer* m_gpu_heap = nullptr;

		tlsf::Allocator m_heap_allocator;
		Vector<Command> m_commands;

		Vector<Chunk> m_chunks;

	private:
		void* heap_allocator(usize size);
		void execute_command(RHIContext* ctx, const Command& command);
		void execute_commands(RHIContext* ctx);

		Chunk* prepare_chunk(Chunk* chunk, u32 required_space = 1);
		Chunk* find_chunk(Material* material, u32 required_space = 1);
		Chunk* find_chunk(MaterialInterface* material, u32 required_space = 1);
		RenderScene& release_chunk(Chunk* chunk);

		inline u32 chunk_index(Chunk* chunk) const { return chunk - m_chunks.data(); }

	public:
		WorldEnvironment environment;

		RenderScene();
		~RenderScene();

		RenderScene& flush(RHIContext* ctx);

		u32 allocate(u32 size, const void* data = nullptr);
		RenderScene& free(u32 address);

		RenderScene& update(u32 address, u32 size);
		RenderScene& update(u32 address, const void* data, u32 size);

		u32 create_geometry(const Geometry& geometry);
		const Geometry& geometry(u32 address) const;
		RenderScene& release_geometry(u32 address);

		u32 create_primitive(const Primitive* desc, u32 count = 1);
		inline u32 create_primitive(const Primitive& desc) { return create_primitive(&desc, 1); }
		const Primitive& primitive(u32 address, u32 idx = 0) const;
		RenderScene& release_primitive(u32 address);

		inline void* map(u32 address) { return m_cpu_heap.data() + address; }
		inline const void* map(u32 address) const { return m_cpu_heap.data() + address; }

		template<typename T>
		inline T* map(u32 address)
		{
			return static_cast<T*>(map(address));
		}

		template<typename T>
		inline const T* map(u32 address) const
		{
			return static_cast<const T*>(map(address));
		}

		inline RHIBuffer* heap() const { return m_gpu_heap; }
		inline const Vector<Chunk>& chunks() const { return m_chunks; }
	};
}// namespace Trinex
