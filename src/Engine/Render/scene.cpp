#include <Core/etl/algorithm.hpp>
#include <Core/math/box.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene.hpp>
#include <Engine/frustum.hpp>
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

	RenderScene::RenderScene()
	{
		m_cpu.scene.resize(1);
		m_gpu.scene = create_buffer<GPUScene>(1);
		m_heap_allocator.init(scene_heap_allocator, &m_cpu.heap);
		init();
	}

	RenderScene::~RenderScene()
	{
		m_gpu.heap->release();
		m_gpu.geometries->release();
		m_gpu.primitives->release();
		m_gpu.scene->release();
	}

	void* RenderScene::scene_heap_allocator(std::size_t size, void* userdata)
	{
		Vector<HeapPage>* buffer = static_cast<Vector<HeapPage>*>(userdata);

		if (buffer->storage().size() < size)
		{
			size = align_up(size, sizeof(HeapPage)) / sizeof(HeapPage);
			buffer->resize(size);
		}

		return buffer->data();
	}

	void RenderScene::execute_command(RHIContext* ctx, const Command& command)
	{
		trinex_profile_cpu_n("RenderScene::execute_command");
		RHIBufferCopy region = {
		        .size       = command.end - command.begin,
		        .dst_offset = command.begin,
		        .src_offset = command.begin,
		};

		ctx->update(*command.dst, *command.src, region);
	}

	void RenderScene::execute_commands(RHIContext* ctx)
	{
		trinex_profile_cpu_n("RenderScene::execute_commands");
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
				execute_command(ctx, batch);

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

	void RenderScene::init(const VectorStorage& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor)
	{
		gpu        = create_buffer(cpu.capacity());
		descriptor = gpu->as_uav(RHIBufferViewType::Structured)->descriptor();

		// Update buffer
		m_commands.push_back(Command{
		        .dst   = &gpu,
		        .src   = &cpu.start,
		        .begin = 0,
		        .end   = static_cast<u32>(cpu.capacity()),
		});

		// Update scene
		m_commands.push_back(Command{
		        .dst   = &m_gpu.scene,
		        .src   = &m_cpu.scene.storage().start,
		        .begin = 0,
		        .end   = sizeof(m_cpu.scene),
		});
	}

	void RenderScene::sync(const VectorStorage& cpu, RHIBuffer*& gpu, RHIDescriptor& descriptor)
	{
		if (cpu.capacity() != gpu->size())
		{
			gpu->release();
			init(cpu, gpu, descriptor);
		}
	}

	RenderScene& RenderScene::flush(RHIContext* ctx)
	{
		trinex_profile_cpu_n("RenderScene::flush");
		sync();
		execute_commands(ctx);
		return *this;
	}

	u32 RenderScene::create_heap_resource(u32 size, const void* data)
	{
		void* ptr   = m_heap_allocator.alloc(size, 16);
		u32 address = static_cast<u8*>(ptr) - m_cpu.heap.data()->as<u8>();

		if (data)
		{
			update_heap_resource(address, data, size);
		}

		return address;
	}

	void* RenderScene::heap_resource(u32 address)
	{
		return m_cpu.heap.data()->as<u8>() + address;
	}

	const void* RenderScene::heap_resource(u32 address) const
	{
		return m_cpu.heap.data()->as<u8>() + address;
	}

	RenderScene& RenderScene::update_heap_resource(u32 address, u32 size)
	{
		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.heap,
		        .src   = &m_cpu.heap.storage().start,
		        .begin = address,
		        .end   = address + size,
		});

		return *this;
	}

	RenderScene& RenderScene::update_heap_resource(u32 address, const void* data, u32 size)
	{
		trinex_profile_cpu_n("RenderScene::update_transform");

		memcpy(m_cpu.heap.data()->as<u8>() + address, data, size);

		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.heap,
		        .src   = &m_cpu.heap.storage().start,
		        .begin = address,
		        .end   = address + size,
		});

		return *this;
	}

	RenderScene& RenderScene::release_heap_resource(u32 address)
	{
		m_heap_allocator.free(m_cpu.heap.data()->as<u8>() + address);
		return *this;
	}

	u32 RenderScene::create_geometry(const Geometry& geometry)
	{
		const u32 id     = m_cpu.geometries.emplace(geometry);
		const u32 offset = id * sizeof(Geometry);

		memcpy(m_cpu.geometries.data() + id, &geometry, sizeof(Geometry));

		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.geometries,
		        .src   = &m_cpu.geometries.storage().start,
		        .begin = offset,
		        .end   = offset + static_cast<u32>(sizeof(Geometry)),
		});
		return id;
	}

	const RenderScene::Geometry& RenderScene::geometry(u32 id) const
	{
		return m_cpu.geometries[id];
	}

	RenderScene& RenderScene::release_geometry(u32 id)
	{
		m_cpu.geometries.erase(id);
		return *this;
	}

	u32 RenderScene::create_primitive(const Primitive& primitive)
	{
		const u32 id     = m_cpu.primitives.emplace(primitive);
		const u32 offset = id * sizeof(Primitive);

		memcpy(m_cpu.primitives.data() + id, &primitive, sizeof(Primitive));

		m_commands.emplace_back(Command{
		        .dst   = &m_gpu.primitives,
		        .src   = &m_cpu.primitives.storage().start,
		        .begin = offset,
		        .end   = offset + static_cast<u32>(sizeof(Primitive)),
		});

		return id;
	}

	const RenderScene::Primitive& RenderScene::primitive(u32 id) const
	{
		return m_cpu.primitives[id];
	}

	RenderScene& RenderScene::release_primitive(u32 id)
	{
		m_cpu.primitives.erase(id);
		return *this;
	}
}// namespace Trinex
