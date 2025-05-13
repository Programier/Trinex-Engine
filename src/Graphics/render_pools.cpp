#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>

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

	static inline Identifier static_calculate_surface_id(SurfaceFormat format, Vector2u size)
	{
		union SurfaceID
		{
			Identifier id;

			struct Value {
				SurfaceFormat format;
				uint16_t x;
				uint16_t y;
			} value;

			static_assert(sizeof(Value) == sizeof(Identifier));
		};

		SurfaceID id;
		id.value.format = format;
		id.value.x      = size.x;
		id.value.y      = size.y;
		return id.id;
	}

	static inline Identifier static_calculate_buffer_id(uint32_t size, BufferCreateFlags flags)
	{
		return (static_cast<Identifier>(size) << 32) | flags.bitfield;
	}

	RHIBufferPool* RHIBufferPool::global_instance()
	{
		static RHIBufferPool pool;
		return &pool;
	}

	RHIBufferPool& RHIBufferPool::update(float dt)
	{
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

	RHI_Buffer* RHIBufferPool::request_buffer(uint32_t size, BufferCreateFlags flags)
	{
		if (size == 0)
			return nullptr;

		flags |= BufferCreateFlags::Dynamic;
		const Identifier buffer_id = static_calculate_buffer_id(size, flags);
		auto& pool                 = m_pools[buffer_id];

		if (!pool.empty())
		{
			RHI_Buffer* buffer = pool.back().buffer;
			pool.pop_back();
			return buffer;
		}

		RHI_Buffer* buffer  = rhi->create_buffer(size, nullptr, flags);
		m_buffer_id[buffer] = buffer_id;
		return buffer;
	}

	RHI_Buffer* RHIBufferPool::request_transient_buffer(uint32_t size, BufferCreateFlags flags)
	{
		if (auto buffer = request_buffer(size, flags))
		{
			render_thread()->call([buffer, this]() { return_buffer(buffer); });
			return buffer;
		}
		return nullptr;
	}

	RHIBufferPool& RHIBufferPool::release_all()
	{
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

	RHIBufferPool& RHIBufferPool::return_buffer(RHI_Buffer* buffer)
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

	RHISurfacePool& RHISurfacePool::update(float dt)
	{
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

	RHI_Texture2D* RHISurfacePool::request_surface(SurfaceFormat format, Vector2u size)
	{
		if (size.x == 0 || size.y == 0)
			return nullptr;

		const Identifier surface_id = static_calculate_surface_id(format, size);
		auto& pool                  = m_pools[surface_id];

		if (!pool.empty())
		{
			RHI_Texture2D* surface = pool.back().surface;
			pool.pop_back();
			return surface;
		}

		TextureCreateFlags flags = TextureCreateFlags::ShaderResource;

		if (format.is_color())
		{
			flags |= TextureCreateFlags::RenderTarget;
			flags |= TextureCreateFlags::UnorderedAccess;
		}
		else if (format.has_depth())
		{
			flags |= TextureCreateFlags::DepthStencilTarget;
		}

		RHI_Texture2D* surface = rhi->create_texture_2d(format, size, 1, flags);
		m_surface_id[surface]  = surface_id;
		return surface;
	}

	RHI_Texture2D* RHISurfacePool::request_transient_surface(SurfaceFormat format, Vector2u size)
	{
		if (auto surface = request_surface(format, size))
		{
			render_thread()->call([surface, this]() { return_surface(surface); });
			return surface;
		}
		return nullptr;
	}

	RHISurfacePool& RHISurfacePool::release_all()
	{
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

	RHISurfacePool& RHISurfacePool::return_surface(RHI_Texture2D* surface)
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

	RenderSurfacePool& RenderSurfacePool::update(float dt)
	{
		for (auto& [id, pool] : m_pools)
		{
			size_t erase_count = 0;

			for (auto& entry : pool)
			{
				if (--entry.frame == 0)
				{
					++erase_count;

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

		render_thread()->call([dt]() { RHISurfacePool::global_instance()->update(dt); });
		return *this;
	}

	RenderSurface* RenderSurfacePool::request_surface(SurfaceFormat format, Vector2u size)
	{
		if (size.x == 0 || size.y == 0)
			return nullptr;

		auto& pool = m_pools[static_calculate_surface_id(format, size)];

		if (!pool.empty())
		{
			RenderSurface* surface = pool.back().surface;
			pool.pop_back();
			return surface;
		}

		RenderSurface* surface = Object::new_instance<RenderSurface>();
		surface->init(format, size);

#if TRINEX_DEBUG_BUILD
		surface->rename(Strings::format("RenderSurface.{}", static_cast<void*>(surface)));
		surfaces_package()->add_object(surface);
#endif
		return surface;
	}

	RenderSurface* RenderSurfacePool::request_transient_surface(SurfaceFormat format, Vector2u size)
	{
		if (auto surface = request_surface(format, size))
		{
			logic_thread()->call([object = Pointer(surface), this]() { return_surface(object.ptr()); });
			return surface;
		}
		return nullptr;
	}

	RenderSurfacePool& RenderSurfacePool::return_surface(RenderSurface* surface)
	{
		auto& pool    = m_pools[static_calculate_surface_id(surface->format(), surface->size())];
		auto& entry   = pool.emplace_back();
		entry.surface = surface;
		entry.frame   = s_resource_live_threshold;
		return *this;
	}

	static class : public TickableObject
	{
		TickableObject& update(float dt) override
		{
			RenderSurfacePool::global_instance()->update(dt);

			render_thread()->call([dt]() {
				RHISurfacePool::global_instance()->update(dt);
				RHIBufferPool::global_instance()->update(dt);
			});
			return *this;
		}
	} m_pool_update;


	static void on_destroy()
	{
		render_thread()->call([]() {
			RHIBufferPool::global_instance()->release_all();
			RHISurfacePool::global_instance()->release_all();
		});

		render_thread()->wait();
	}

	static DestroyController destroy_controller(on_destroy);

}// namespace Engine
