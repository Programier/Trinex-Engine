#pragma once
#include <Core/etl/range_allocator.hpp>
#include <Engine/enviroment.hpp>
#include <RHI/types.hpp>

namespace Trinex
{
	class PrimitiveComponent;
	class LightComponent;
	class PostProcessComponent;
	struct Frustum;
	class RHIContext;
	class RHIBuffer;

	class ENGINE_EXPORT Scene final
	{
	public:
		static constexpr u32 s_init_transforms_count = 4096;
		static constexpr u32 s_init_geometries_count = 1024;
		static constexpr u32 s_init_primitives_count = 2048;

		using Transform = Matrix4f;

		struct Geometry {
			struct Buffer {
				RHIDeviceAddress address;
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
			u32 transform;
			u32 material;
			u32 geometry;
			u32 flags;
		};

	private:
		struct GPUScene {
			RHIDescriptor transforms;
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
				return begin < other.end;
			}
		};

		struct {
			Vector<Transform> transforms;
			Vector<Geometry> geometries;
			Vector<Primitive> primitives;
			Vector<GPUScene> scene;
		} m_cpu;

		struct {
			RHIBuffer* transforms = nullptr;
			RHIBuffer* geometries = nullptr;
			RHIBuffer* primitives = nullptr;
			RHIBuffer* scene      = nullptr;
		} m_gpu;

		struct Allocator {
			RangeAllocator<8> transforms;
			RangeAllocator<1> geometries;
			RangeAllocator<1> primitives;

			template<u32 bins, typename T>
			static typename RangeAllocator<bins>::Range allocate(RangeAllocator<bins>& allocator, Vector<T>& buffer, u32 count)
			{
				while (true)
				{
					if (auto range = allocator.alloc(count))
					{
						return range;
					}

					u32 size = buffer.size();
					buffer.resize(size * 2);
					allocator.free({.offset = size, .size = size});
				}
			}
		} m_alloc;

		Vector<Command> m_commands;

	private:
		static void execute_command(RHIContext* ctx, const Command& command);
		void execute_commands(RHIContext* ctx);

		void init(usize size, void* const& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor);
		void sync(usize size, void* const& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor);

		template<typename T>
		void init(Vector<T>& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor)
		{
			init(cpu.size() * sizeof(T), cpu.storage().start, gpu, descriptor);
		}

		template<typename T>
		void sync(Vector<T>& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor)
		{
			sync(cpu.size() * sizeof(T), cpu.storage().start, gpu, descriptor);
		}

		inline void init()
		{
			init(m_cpu.transforms, m_gpu.transforms, m_cpu.scene[0].transforms);
			init(m_cpu.geometries, m_gpu.geometries, m_cpu.scene[0].geometries);
			init(m_cpu.primitives, m_gpu.primitives, m_cpu.scene[0].primitives);
		}

		inline void sync()
		{
			sync(m_cpu.transforms, m_gpu.transforms, m_cpu.scene[0].transforms);
			sync(m_cpu.geometries, m_gpu.geometries, m_cpu.scene[0].geometries);
			sync(m_cpu.primitives, m_gpu.primitives, m_cpu.scene[0].primitives);
		}

	public:
		WorldEnvironment environment;

		Scene();
		~Scene();

		Scene& flush(RHIContext* ctx);

		u32 create_transform(u32 count = 1, const Transform* transforms = nullptr);
		const Transform* transform(u32 id) const;
		Scene& update_transform(u32 id, const Transform* transforms, u32 count = 1);
		Scene& update_transform(u32 id, const Transform& transforms);
		Scene& release_transform(u32 id, u32 count);

		u32 create_geometry(const Geometry& geometry);
		const Geometry& geometry(u32 id) const;
		Scene& release_geometry(u32 id);

		u32 create_primitive(const Primitive& primitive);
		const Primitive& primitive(u32 id) const;
		Scene& release_primitive(u32 id);

		inline RHIBuffer* scene_buffer() const { return m_gpu.scene; }

		u32 add_primitive(PrimitiveComponent* primitive, const Box3i& box);
		Scene& update_primitive(u32 id, const Box3i& box);
		Scene& remove_primitive(u32 id);

		u32 add_light(LightComponent* light, const Box3i& box);
		Scene& update_light(u32 id, const Box3i& box);
		Scene& remove_light(u32 id);

		u32 add_post_process(PostProcessComponent* post_process, const Box3i& box);
		Scene& update_post_process(u32 id, const Box3i& box);
		Scene& remove_post_process(u32 id);
	};
}// namespace Trinex
