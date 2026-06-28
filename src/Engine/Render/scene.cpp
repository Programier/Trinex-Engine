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
#include <Graphics/material.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	RenderScene::RenderScene()
	{
		static auto callback = +[](std::size_t size, void* userdata) -> void* {
			return static_cast<RenderScene*>(userdata)->heap_allocator(size);
		};

		m_heap_allocator.init(callback, this);
	}

	RenderScene::~RenderScene()
	{
		if (m_gpu_heap)
			m_gpu_heap->release();
	}

	void* RenderScene::heap_allocator(usize size)
	{
		if (m_cpu_heap.size() < size)
		{
			if (m_cpu_heap.size() > 0)
			{
				m_commands.push_back({
				        .begin = 0,
				        .end   = static_cast<u32>(m_cpu_heap.size()),
				});
			}

			size = align_up(size, 4096);
			m_cpu_heap.resize(size);
		}

		return m_cpu_heap.data();
	}

	void RenderScene::execute_command(RHIContext* ctx, const Command& command)
	{
		trinex_profile_cpu_n("RenderScene::execute_command");
		RHIBufferCopy region = {
		        .size       = command.end - command.begin,
		        .dst_offset = command.begin,
		        .src_offset = command.begin,
		};

		ctx->update(m_gpu_heap, m_cpu_heap.data(), region);
	}

	void RenderScene::execute_commands(RHIContext* ctx)
	{
		trinex_profile_cpu_n("RenderScene::execute_commands");

		if (m_gpu_heap == nullptr && m_cpu_heap.size() > 0)
		{
			RHIBufferFlags flags = RHIBufferFlags::DeviceAddress | RHIBufferFlags::TransferDst |
			                       RHIBufferFlags::ByteAddressBuffer | RHIBufferFlags::ShaderResource;
			m_gpu_heap = RHI::instance()->create_buffer(m_cpu_heap.size(), flags);
		}

		if (m_commands.empty())
			return;

		static constexpr u32 s_merge_threshold = 512;

		etl::sort(m_commands.begin(), m_commands.end());

		Command batch = m_commands.front();
		ctx->barrier(m_gpu_heap, RHIAccess::TransferDst);

		for (size_t i = 1; i < m_commands.size(); ++i)
		{
			const Command& cmd         = m_commands[i];
			const bool is_close_enough = cmd.begin <= batch.end + s_merge_threshold;

			if (is_close_enough)
			{
				batch.end = Math::max(batch.end, cmd.end);
			}
			else
			{
				execute_command(ctx, batch);
				batch = cmd;
			}
		}

		execute_command(ctx, batch);
		ctx->barrier(m_gpu_heap, RHIAccess::SRVGraphics | RHIAccess::SRVCompute);

		m_commands.clear();
	}

	RenderScene::Chunk* RenderScene::prepare_chunk(RenderScene::Chunk* chunk, u32 required_space)
	{
		const u32 required = align_up(chunk->count + required_space, 128);

		if (required > chunk->capacity)
		{
			const u32 data = chunk->address;

			chunk->address = allocate(required * sizeof(u32));

			if (data)
			{
				auto src = map(data);
				auto dst = map(chunk->address);

				etl::memcpy(dst, src, chunk->capacity * sizeof(u32));
				free(data);
			}

			chunk->capacity = required;
		}

		return chunk;
	}

	RenderScene::Chunk* RenderScene::find_chunk(Material* material, u32 required_space)
	{
		for (Chunk& chunk : m_chunks)
		{
			if (chunk.material == material)
				return prepare_chunk(&chunk, required_space);
		}

		Chunk* chunk    = &m_chunks.emplace_back();
		chunk->material = material;
		return prepare_chunk(chunk, required_space);
	}

	RenderScene::Chunk* RenderScene::find_chunk(MaterialInterface* material, u32 required_space)
	{
		return material ? find_chunk(material->material(), required_space)
		                : find_chunk(static_cast<Material*>(nullptr), required_space);
	}

	RenderScene& RenderScene::release_chunk(Chunk* chunk)
	{
		const u32 idx    = chunk_index(chunk);
		const usize size = m_chunks.size();

		if (idx >= size)
			return *this;

		if (chunk->address)
		{
			free(chunk->address);
		}

		if (idx != size - 1)
		{
			m_chunks[idx] = m_chunks.back();
		}

		m_chunks.pop_back();
		return *this;
	}

	RenderScene& RenderScene::flush(RHIContext* ctx)
	{
		trinex_profile_cpu_n("RenderScene::flush");
		execute_commands(ctx);
		return *this;
	}

	u32 RenderScene::allocate(u32 size, const void* data)
	{
		void* ptr   = m_heap_allocator.alloc(size, 16);
		u32 address = static_cast<u8*>(ptr) - m_cpu_heap.data();

		if (data)
		{
			update(address, data, size);
		}

		return address;
	}

	RenderScene& RenderScene::free(u32 address)
	{
		m_heap_allocator.free(map(address));
		return *this;
	}

	RenderScene& RenderScene::update(u32 address, u32 size)
	{
		m_commands.emplace_back(Command{
		        .begin = address,
		        .end   = address + size,
		});

		return *this;
	}

	RenderScene& RenderScene::update(u32 address, const void* data, u32 size)
	{
		trinex_profile_cpu_n("RenderScene::update_transform");
		memcpy(map(address), data, size);

		m_commands.emplace_back(Command{
		        .begin = address,
		        .end   = address + size,
		});

		return *this;
	}

	u32 RenderScene::create_geometry(const Geometry& geometry)
	{
		return allocate(sizeof(Geometry), &geometry);
	}

	const RenderScene::Geometry& RenderScene::geometry(u32 address) const
	{
		return *static_cast<const Geometry*>(map(address));
	}

	RenderScene& RenderScene::release_geometry(u32 address)
	{
		return free(address);
	}

	u32 RenderScene::create_primitive(const Primitive* desc, u32 count)
	{
		if (count == 0)
			return 0;

		Chunk* chunk = find_chunk(desc->material, count);
		u32* indices = map<u32>(chunk->address) + chunk->count;

		u32 address          = allocate(sizeof(Primitive) * count);
		Primitive* instances = map<Primitive>(address);

		for (u32 i = 0; i < count; ++i)
		{
			instances[i] = desc[i];
			instances[i].flags &= Primitive::UserMask;
			instances[i].chunk = chunk->count + i;

			indices[i] = address + sizeof(Primitive) * i;
		}

		instances[count - 1].flags |= Primitive::IsLast;

		update(address, sizeof(Primitive) * count);
		update(chunk->address + chunk->count * sizeof(u32), sizeof(u32) * count);
		chunk->count += count;

		return address;
	}

	const RenderScene::Primitive& RenderScene::primitive(u32 address, u32 idx) const
	{
		return *static_cast<const Primitive*>(map(address + sizeof(Primitive) * idx));
	}

	RenderScene& RenderScene::release_primitive(u32 address)
	{
		Primitive* instance = map<Primitive>(address);

		while (true)
		{
			Chunk* chunk   = find_chunk(instance->material);
			const u32 last = --chunk->count;

			if (last == 0)
			{
				release_chunk(chunk);
			}
			else if (instance->chunk != last)
			{
				u32* src = map<u32>(chunk->address + last * sizeof(u32));
				update(chunk->address + instance->chunk * sizeof(u32), src, sizeof(address));
			}

			if (instance->flags & Primitive::IsLast)
			{
				break;
			}
			else
			{
				++instance;
			}
		}

		return free(address);
	}
}// namespace Trinex
