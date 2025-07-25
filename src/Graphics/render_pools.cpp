#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
#if TRINEX_DEBUG_BUILD
	static Package* surfaces_package()
	{
		static Package* package = Package::static_find_package("TrinexEngine::SurfacePool", true);
		return package;
	}
#endif

	static constexpr uint64_t s_resource_live_threshold = 60 * 3;

	static inline Identifier static_calculate_surface_id(RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags)
	{
		union SurfaceID
		{
			Identifier id = 0;

			struct Value {
				uint16_t x;
				uint16_t y;
				RHISurfaceFormat::Enum format : 16;
				RHITextureCreateFlags flags;
			} value;

			static_assert(sizeof(Value) <= sizeof(Identifier));
		};

		SurfaceID id;
		id.value.format = format;
		id.value.x      = size.x;
		id.value.y      = size.y;
		id.value.flags  = flags;
		return id.id;
	}

	static inline Identifier static_calculate_buffer_id(uint32_t size, RHIBufferCreateFlags flags)
	{
		return (static_cast<Identifier>(size) << 32) | flags.bitfield;
	}

	RHIFencePool* RHIFencePool::global_instance()
	{
		static RHIFencePool pool;
		return &pool;
	}

	RHIFencePool& RHIFencePool::flush_transient()
	{
		for (RHIFence* fence : m_transient_fences)
		{
			return_fence(fence);
		}
		m_transient_fences.clear();
		return *this;
	}

	RHIFencePool& RHIFencePool::update()
	{
		flush_transient();

		size_t erase_count = 0;

		for (auto& entry : m_pool)
		{
			if (--entry.frame == 0)
			{
				++erase_count;
				entry.fence->release();
			}
		}

		if (erase_count > 0)
		{
			m_pool.erase(m_pool.begin(), m_pool.begin() + erase_count);
		}

		return *this;
	}

	RHIFence* RHIFencePool::request_fence()
	{
		if (!m_pool.empty())
		{
			RHIFence* fence = m_pool.back().fence;
			fence->reset();
			m_pool.pop_back();
			return fence;
		}
		return rhi->create_fence();
	}

	RHIFence* RHIFencePool::request_transient_fence()
	{
		if (auto fence = request_fence())
		{
			m_transient_fences.push_back(fence);
			return fence;
		}
		return nullptr;
	}

	RHIFencePool& RHIFencePool::release_all()
	{
		flush_transient();

		for (auto& entry : m_pool)
		{
			entry.fence->release();
		}
		m_pool.clear();
		return *this;
	}

	RHIFencePool& RHIFencePool::return_fence(RHIFence* fence)
	{
		auto& entry = m_pool.emplace_back();
		entry.fence = fence;
		entry.frame = s_resource_live_threshold;
		return *this;
	}

	RHIBufferPool* RHIBufferPool::global_instance()
	{
		static RHIBufferPool pool;
		return &pool;
	}

	RHIBufferPool& RHIBufferPool::flush_transient()
	{
		for (RHIBuffer* buffer : m_transient_buffers)
		{
			return_buffer(buffer);
		}

		m_transient_buffers.clear();
		return *this;
	}

	RHIBufferPool& RHIBufferPool::update()
	{
		flush_transient();

		for (auto& [id, pool] : m_pools)
		{
			size_t erase_count = 0;

			for (auto& entry : pool)
			{
				if (--entry.frame == 0)
				{
					++erase_count;
					entry.buffer->release();
					m_buffer_id.erase(entry.buffer);
				}
			}

			if (erase_count > 0)
			{
				pool.erase(pool.begin(), pool.begin() + erase_count);
			}
		}
		return *this;
	}

	RHIBuffer* RHIBufferPool::request_buffer(uint32_t size, RHIBufferCreateFlags flags)
	{
		if (size == 0)
			return nullptr;

		flags |= RHIBufferCreateFlags::Dynamic;
		const Identifier buffer_id = static_calculate_buffer_id(size, flags);
		auto& pool                 = m_pools[buffer_id];

		if (!pool.empty())
		{
			RHIBuffer* buffer = pool.back().buffer;
			pool.pop_back();
			return buffer;
		}

		RHIBuffer* buffer  = rhi->create_buffer(size, nullptr, flags);
		m_buffer_id[buffer] = buffer_id;
		return buffer;
	}

	RHIBuffer* RHIBufferPool::request_transient_buffer(uint32_t size, RHIBufferCreateFlags flags)
	{
		if (auto buffer = request_buffer(size, flags))
		{
			m_transient_buffers.push_back(buffer);
			return buffer;
		}
		return nullptr;
	}

	RHIBufferPool& RHIBufferPool::release_all()
	{
		flush_transient();

		m_buffer_id.clear();
		for (auto& [id, pool] : m_pools)
		{
			for (auto& entry : pool)
			{
				entry.buffer->release();
			}
		}
		m_pools.clear();
		return *this;
	}

	RHIBufferPool& RHIBufferPool::return_buffer(RHIBuffer* buffer)
	{
		auto it = m_buffer_id.find(buffer);

		if (it == m_buffer_id.end())
			return *this;

		auto& pool   = m_pools[it->second];
		auto& entry  = pool.emplace_back();
		entry.buffer = buffer;
		entry.frame  = s_resource_live_threshold;
		return *this;
	}

	RHISurfacePool* RHISurfacePool::global_instance()
	{
		static RHISurfacePool pool;
		return &pool;
	}

	RHISurfacePool& RHISurfacePool::flush_transient()
	{
		for (RHITexture* texture : m_transient_textures)
		{
			return_surface(texture);
		}
		m_transient_textures.clear();
		return *this;
	}

	RHISurfacePool& RHISurfacePool::update()
	{
		flush_transient();

		for (auto& [id, pool] : m_pools)
		{
			size_t erase_count = 0;

			for (auto& entry : pool)
			{
				if (--entry.frame == 0)
				{
					++erase_count;
					entry.surface->release();
					m_surface_id.erase(entry.surface);
				}
			}

			if (erase_count > 0)
			{
				pool.erase(pool.begin(), pool.begin() + erase_count);
			}
		}

		return *this;
	}

	RHITexture* RHISurfacePool::request_surface(RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags)
	{
		if (size.x == 0 || size.y == 0)
			return nullptr;

		flags |= RHITextureCreateFlags::ShaderResource;

		if (format.is_color())
		{
			flags |= RHITextureCreateFlags::RenderTarget;
		}
		else if (format.has_depth())
		{
			flags |= RHITextureCreateFlags::DepthStencilTarget;
		}

		const Identifier surface_id = static_calculate_surface_id(format, size, flags);
		auto& pool                  = m_pools[surface_id];

		if (!pool.empty())
		{
			RHITexture* surface = pool.back().surface;
			pool.pop_back();
			return surface;
		}

		RHITexture* surface  = rhi->create_texture(RHITextureType::Texture2D, RHIColorFormat(format), {size, 1}, 1, flags);
		m_surface_id[surface] = surface_id;
		return surface;
	}

	RHITexture* RHISurfacePool::request_transient_surface(RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags)
	{
		if (auto surface = request_surface(format, size, flags))
		{
			m_transient_textures.push_back(surface);
			return surface;
		}
		return nullptr;
	}

	RHISurfacePool& RHISurfacePool::release_all()
	{
		flush_transient();

		m_surface_id.clear();
		for (auto& [id, pool] : m_pools)
		{
			for (auto& entry : pool)
			{
				entry.surface->release();
			}
		}
		m_pools.clear();
		return *this;
	}

	RHISurfacePool& RHISurfacePool::return_surface(RHITexture* surface)
	{
		auto it = m_surface_id.find(surface);

		if (it == m_surface_id.end())
			return *this;

		auto& pool    = m_pools[it->second];
		auto& entry   = pool.emplace_back();
		entry.surface = surface;
		entry.frame   = s_resource_live_threshold;
		return *this;
	}

	RenderSurfacePool* RenderSurfacePool::global_instance()
	{
		static RenderSurfacePool pool;
		return &pool;
	}

	RenderSurfacePool& RenderSurfacePool::flush_transient()
	{
		for (RenderSurface* surface : m_transient_surfaces)
		{
			return_surface(surface);
		}
		m_transient_surfaces.clear();
		return *this;
	}

	RenderSurfacePool& RenderSurfacePool::update()
	{
		flush_transient();

		for (auto& [id, pool] : m_pools)
		{
			size_t erase_count = 0;

			for (auto& entry : pool)
			{
				if (--entry.frame == 0)
				{
					++erase_count;
					entry.surface->remove_reference();
#if TRINEX_DEBUG_BUILD
					if (entry.surface->owner() == surfaces_package())
					{
						entry.surface->owner(nullptr);
					}
#endif
				}
			}

			if (erase_count > 0)
			{
				pool.erase(pool.begin(), pool.begin() + erase_count);
			}
		}
		return *this;
	}

	RenderSurface* RenderSurfacePool::request_surface(RHISurfaceFormat format, Vector2u size)
	{
		if (size.x == 0 || size.y == 0)
			return nullptr;

		auto& pool = m_pools[static_calculate_surface_id(format, size, {})];

		if (!pool.empty())
		{
			RenderSurface* surface = pool.back().surface;
			pool.pop_back();
			return surface;
		}

		RenderSurface* surface = Object::new_instance<RenderSurface>();
		surface->add_reference();
		surface->init(format, size);

#if TRINEX_DEBUG_BUILD
		surface->rename(Strings::format("RenderSurface.{}", static_cast<void*>(surface)));
		surfaces_package()->add_object(surface);
#endif
		return surface;
	}

	RenderSurface* RenderSurfacePool::request_transient_surface(RHISurfaceFormat format, Vector2u size)
	{
		if (auto surface = request_surface(format, size))
		{
			m_transient_surfaces.push_back(surface);
			return surface;
		}
		return nullptr;
	}

	RenderSurfacePool& RenderSurfacePool::return_surface(RenderSurface* surface)
	{
		auto& pool    = m_pools[static_calculate_surface_id(surface->format(), surface->size(), {})];
		auto& entry   = pool.emplace_back();
		entry.surface = surface;
		entry.frame   = s_resource_live_threshold;
		return *this;
	}

	RHITimestampPool* RHITimestampPool::global_instance()
	{
		static RHITimestampPool pool;
		return &pool;
	}

	RHITimestampPool& RHITimestampPool::update()
	{
		size_t erase_count = 0;

		for (auto& entry : m_pool)
		{
			if (--entry.frame == 0)
			{
				++erase_count;
				entry.timestamp->release();
			}
		}

		if (erase_count > 0)
		{
			m_pool.erase(m_pool.begin(), m_pool.begin() + erase_count);
		}

		return *this;
	}

	RHITimestamp* RHITimestampPool::request_timestamp()
	{
		if (!m_pool.empty())
		{
			RHITimestamp* timestamp = m_pool.back().timestamp;
			m_pool.pop_back();
			return timestamp;
		}
		return rhi->create_timestamp();
	}

	RHITimestampPool& RHITimestampPool::release_all()
	{
		for (auto& entry : m_pool)
		{
			entry.timestamp->release();
		}
		m_pool.clear();
		return *this;
	}

	RHITimestampPool& RHITimestampPool::return_timestamp(RHITimestamp* timestamp)
	{
		auto& entry     = m_pool.emplace_back();
		entry.timestamp = timestamp;
		entry.frame     = s_resource_live_threshold;
		return *this;
	}

	RHIPipelineStatisticsPool* RHIPipelineStatisticsPool::global_instance()
	{
		static RHIPipelineStatisticsPool pool;
		return &pool;
	}

	RHIPipelineStatisticsPool& RHIPipelineStatisticsPool::update()
	{
		size_t erase_count = 0;

		for (auto& entry : m_pool)
		{
			if (--entry.frame == 0)
			{
				++erase_count;
				entry.stats->release();
			}
		}

		if (erase_count > 0)
		{
			m_pool.erase(m_pool.begin(), m_pool.begin() + erase_count);
		}

		return *this;
	}

	RHIPipelineStatistics* RHIPipelineStatisticsPool::request_statistics()
	{
		if (!m_pool.empty())
		{
			RHIPipelineStatistics* stats = m_pool.back().stats;
			m_pool.pop_back();
			return stats;
		}
		return rhi->create_pipeline_statistics();
	}

	RHIPipelineStatisticsPool& RHIPipelineStatisticsPool::release_all()
	{
		for (auto& entry : m_pool)
		{
			entry.stats->release();
		}
		m_pool.clear();
		return *this;
	}

	RHIPipelineStatisticsPool& RHIPipelineStatisticsPool::return_statistics(RHIPipelineStatistics* stats)
	{
		auto& entry = m_pool.emplace_back();
		entry.stats = stats;
		entry.frame = s_resource_live_threshold;
		return *this;
	}

	static class : public TickableObject
	{
		TickableObject& update(float) override
		{
			RenderSurfacePool::global_instance()->update();

			render_thread()->call([]() {
				RHISurfacePool::global_instance()->update();
				RHIBufferPool::global_instance()->update();
				RHIFencePool::global_instance()->update();
				RHITimestampPool::global_instance()->update();
				RHIPipelineStatisticsPool::global_instance()->update();
			});
			return *this;
		}
	} m_pool_update;


	static void on_destroy()
	{
		render_thread()->call([]() {
			RHIBufferPool::global_instance()->release_all();
			RHISurfacePool::global_instance()->release_all();
			RHIFencePool::global_instance()->release_all();
			RHITimestampPool::global_instance()->release_all();
			RHIPipelineStatisticsPool::global_instance()->release_all();
		});

		render_thread()->wait();
	}

	static DestroyController destroy_controller(on_destroy);

}// namespace Engine
