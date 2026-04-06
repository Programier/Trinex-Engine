#include <Core/etl/algorithm.hpp>
#include <Core/math/box.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/frustum.hpp>
#include <Engine/scene.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	template<typename T = u8>
	static RHIBuffer* create_buffer(usize size)
	{
		RHIBufferFlags flags = RHIBufferFlags::DeviceAddress | RHIBufferFlags::TransferDst | RHIBufferFlags::StructuredBuffer |
		                       RHIBufferFlags::UnorderedAccess;
		return RHI::instance()->create_buffer(size * sizeof(T), flags);
	}

	Scene::Scene()
	{
		m_cpu.transforms.resize(s_init_transforms_count * sizeof(Transform));
		m_cpu.geometries.resize(s_init_geometries_count * sizeof(Geometry));
		m_cpu.primitives.resize(s_init_primitives_count * sizeof(Primitive));
		m_cpu.scene.resize(1);

		m_alloc.transforms.free({0, s_init_transforms_count});
		m_alloc.geometries.free({0, s_init_geometries_count});
		m_alloc.primitives.free({0, s_init_primitives_count});

		m_gpu.scene = create_buffer<GPUScene>(1);
		init();
	}

	Scene::~Scene()
	{
		m_gpu.transforms->release();
		m_gpu.geometries->release();
		m_gpu.primitives->release();
		m_gpu.scene->release();
	}

	void Scene::execute_command(RHIContext* ctx, const Command& command)
	{
		RHIBufferCopy region = {
		        .size       = command.end - command.begin,
		        .dst_offset = command.begin,
		        .src_offset = command.begin,
		};

		ctx->update(*command.dst, *command.src, region);
	}

	void Scene::execute_commands(RHIContext* ctx)
	{
		if (m_commands.empty())
			return;

		static constexpr u32 s_merge_threshold = 512;

		etl::sort(m_commands.begin(), m_commands.end());

		Command batch = m_commands.front();
		ctx->barrier(*batch.dst, RHIAccess::TransferDst);

		for (size_t i = 1; i < m_commands.size(); ++i)
		{
			const Command& cmd = m_commands[i];

			const bool is_same_dest    = batch.dst == cmd.dst;
			const bool is_close_enough = cmd.begin <= batch.end + s_merge_threshold;

			if (is_same_dest && is_close_enough)
			{
				batch.end = Math::max(batch.end, cmd.end);
			}
			else
			{
				if (!is_same_dest)
				{
					ctx->barrier(*batch.dst, RHIAccess::SRVGraphics | RHIAccess::SRVCompute);
					ctx->barrier(*cmd.dst, RHIAccess::TransferDst);
				}

				batch = cmd;
			}
		}

		execute_command(ctx, batch);
		ctx->barrier(*batch.dst, RHIAccess::SRVGraphics | RHIAccess::SRVCompute);

		m_commands.clear();
	}

	void Scene::init(usize size, void* const& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor)
	{
		gpu        = create_buffer(size);
		descriptor = gpu->as_uav(RHIBufferViewType::Structured)->descriptor();

		// Update buffer
		m_commands.push_back(Command{
		        .dst   = &gpu,
		        .src   = &cpu,
		        .begin = 0,
		        .end   = static_cast<u32>(size),
		});


		// Update scene
		m_commands.push_back(Command{
		        .dst   = &m_gpu.scene,
		        .src   = &m_cpu.scene.storage().start,
		        .begin = 0,
		        .end   = sizeof(m_cpu.scene),
		});
	}


	void Scene::sync(usize size, void* const& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor)
	{
		if (size != gpu->size())
		{
			gpu->release();
			init(size, cpu, gpu, descriptor);
		}
	}

	Scene& Scene::flush(RHIContext* ctx)
	{
		sync();
		execute_commands(ctx);
		return *this;
	}

	u32 Scene::create_transform(u32 count, const Transform* transforms)
	{
		u32 id = m_alloc.allocate(m_alloc.transforms, m_cpu.transforms, count).offset;

		if (transforms)
		{
			update_transform(id, transforms, count);
		}

		return id;
	}

	const Scene::Transform* Scene::transform(u32 id) const
	{
		return m_cpu.transforms.data() + id;
	}

	Scene& Scene::update_transform(u32 id, const Transform* transforms, u32 count)
	{
		u32 offset = id * sizeof(Transform);
		u32 size   = count * sizeof(Transform);

		memcpy(m_cpu.transforms.data() + offset, transforms, size);

		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.transforms,
		        .src   = &m_cpu.transforms.storage().start,
		        .begin = offset,
		        .end   = offset + size,
		});

		return *this;
	}

	Scene& Scene::update_transform(u32 id, const Transform& transforms)
	{
		return update_transform(id, &transforms, 1);
	}

	Scene& Scene::release_transform(u32 id, u32 count)
	{
		m_alloc.transforms.free({id, count});
		return *this;
	}

	u32 Scene::create_geometry(const Geometry& geometry)
	{
		u32 id     = m_alloc.allocate(m_alloc.geometries, m_cpu.geometries, 1).offset;
		u32 offset = id * sizeof(Geometry);

		memcpy(m_cpu.geometries.data() + offset, &geometry, sizeof(Geometry));

		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.geometries,
		        .src   = &m_cpu.geometries.storage().start,
		        .begin = offset,
		        .end   = offset + static_cast<u32>(sizeof(Geometry)),
		});
		return id;
	}

	const Scene::Geometry& Scene::geometry(u32 id) const
	{
		return m_cpu.geometries[id];
	}

	Scene& Scene::release_geometry(u32 id)
	{
		m_alloc.geometries.free({id, 1});
		return *this;
	}

	u32 Scene::create_primitive(const Primitive& primitive)
	{
		u32 id     = m_alloc.allocate(m_alloc.primitives, m_cpu.primitives, 1).offset;
		u32 offset = id * sizeof(Primitive);

		memcpy(m_cpu.primitives.data() + offset, &primitive, sizeof(Primitive));

		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.primitives,
		        .src   = &m_cpu.primitives.storage().start,
		        .begin = offset,
		        .end   = offset + static_cast<u32>(sizeof(Primitive)),
		});

		return id;
	}

	const Scene::Primitive& Scene::primitive(u32 id) const
	{
		return m_cpu.primitives[id];
	}

	Scene& Scene::release_primitive(u32 id)
	{
		m_alloc.primitives.free({id, 1});
		return *this;
	}

	u32 Scene::add_primitive(PrimitiveComponent* primitive, const Box3i& box)
	{
		// trinex_profile_cpu_n("Scene::add_primitive");
		// return m_primitive_octree.insert(primitive, box);
		return -1;
	}

	Scene& Scene::update_primitive(u32 id, const Box3i& box)
	{
		trinex_profile_cpu_n("Scene::update_primitive");


		return *this;
	}

	Scene& Scene::remove_primitive(u32 id)
	{
		trinex_profile_cpu_n("Scene::remove_primitive");
		return *this;
	}


	u32 Scene::add_light(LightComponent* light, const Box3i& box)
	{
		return -1;
	}

	Scene& Scene::update_light(u32 id, const Box3i& box)
	{
		// if (light->light_type() != LightComponent::Directional)
		// {
		// 	remove_light(light);
		// 	light->update_bounding_box();
		// 	add_light(light);
		// }
		return *this;
	}

	Scene& Scene::remove_light(u32 id)
	{
		// if (light->light_type() == LightComponent::Directional)
		// {
		// 	m_directional_lights.erase(light);
		// }
		//else
		// {
		// 	m_light_octree.remove(id);
		// }

		return *this;
	}

	u32 Scene::add_post_process(PostProcessComponent* post_process, const Box3i& box)
	{
		return -1;
	}

	Scene& Scene::update_post_process(u32 id, const Box3i& box)
	{
		return *this;
	}

	Scene& Scene::remove_post_process(u32 id)
	{
		return *this;
	}
}// namespace Trinex
