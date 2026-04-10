#pragma once
#include <Core/etl/sparce_vector.hpp>
#include <Core/math/matrix.hpp>
#include <Engine/enviroment.hpp>
#include <RHI/types.hpp>
#include <tlfs/tlsf.hpp>

namespace Trinex
{
	class PrimitiveComponent;
	class LightComponent;
	class PostProcessComponent;
	struct Frustum;
	class RHIContext;
	class RHIBuffer;

	class ENGINE_EXPORT RenderScene final
	{
	public:
		static constexpr u32 s_init_geometries_count = 1024;
		static constexpr u32 s_init_primitives_count = 2048;

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

			Buffer vertex_stream;
			Buffer animation_stream;
			Buffer index_stream;

			u32 vertices;

			Vector3f aabb_min;
			Vector3f aabb_max;
		};

		struct Primitive {
			struct Flags {
				enum Enum : u32
				{
					IsAlive = BIT(0),
				};

				trinex_bitfield_enum_struct(Flags, u32);
			};
			using enum Flags::Enum;

			u32 transform;
			u32 material;
			u32 geometry;
			Flags flags;
		};

	private:
		struct alignas(4096) HeapPage {
			template<typename T>
			T* as()
			{
				return reinterpret_cast<T*>(this);
			}

			template<typename T>
			const T* as() const
			{
				return reinterpret_cast<const T*>(this);
			}
		};
		struct GPUScene {
			RHIDescriptor heap;
			RHIDescriptor geometries;
			RHIDescriptor primitives;
		};

		struct Command {
			RHIBuffer** dst;
			void* const* src;
			u32 begin;
			u32 end;

			inline bool operator<(const Command& other) const
			{
				if (dst != other.dst)
					return dst < other.dst;
				return begin < other.begin;
			}
		};

		struct CPU {
			Vector<HeapPage> heap;
			SparceVector<Geometry, u32> geometries;
			SparceVector<Primitive, u32> primitives;
			Vector<GPUScene> scene;
			Vector<u32> active;

			CPU() : heap(1), geometries(s_init_geometries_count), primitives(s_init_primitives_count) {}
		} m_cpu;

		struct {
			RHIBuffer* heap       = nullptr;
			RHIBuffer* geometries = nullptr;
			RHIBuffer* primitives = nullptr;
			RHIBuffer* scene      = nullptr;
		} m_gpu;

		tlsf::Allocator m_heap_allocator;
		Vector<Command> m_commands;

	private:
		static void* scene_heap_allocator(std::size_t size, void* userdata);
		static void execute_command(RHIContext* ctx, const Command& command);
		void execute_commands(RHIContext* ctx);

		void init(const VectorStorage& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor);
		void sync(const VectorStorage& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor);

		inline void init()
		{
			init(m_cpu.heap.storage(), m_gpu.heap, m_cpu.scene[0].heap);
			init(m_cpu.geometries.storage(), m_gpu.geometries, m_cpu.scene[0].geometries);
			init(m_cpu.primitives.storage(), m_gpu.primitives, m_cpu.scene[0].primitives);
		}

		inline void sync()
		{
			sync(m_cpu.heap.storage(), m_gpu.heap, m_cpu.scene[0].heap);
			sync(m_cpu.geometries.storage(), m_gpu.geometries, m_cpu.scene[0].geometries);
			sync(m_cpu.primitives.storage(), m_gpu.primitives, m_cpu.scene[0].primitives);
		}


	public:
		WorldEnvironment environment;

		RenderScene();
		~RenderScene();

		RenderScene& flush(RHIContext* ctx);

		u32 create_heap_resource(u32 size, const void* data = nullptr);
		void* heap_resource(u32 address);
		const void* heap_resource(u32 address) const;

		RenderScene& update_heap_resource(u32 address, u32 size);
		RenderScene& update_heap_resource(u32 address, const void* data, u32 size);
		RenderScene& release_heap_resource(u32 address);

		u32 create_geometry(const Geometry& geometry);
		const Geometry& geometry(u32 id) const;
		RenderScene& release_geometry(u32 id);

		u32 create_primitive(const Primitive& primitive);
		const Primitive& primitive(u32 id) const;
		RenderScene& release_primitive(u32 id);

		inline RHIBuffer* scene_buffer() const { return m_gpu.scene; }
		inline const tlsf::Allocator& heap_allocator() const { return m_heap_allocator; }
	};
}// namespace Trinex
