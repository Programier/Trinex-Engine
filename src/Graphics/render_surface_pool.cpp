#include <Core/base_engine.hpp>
#include <Core/package.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_surface_pool.hpp>

namespace Engine
{
#if TRINEX_DEBUG_BUILD
	static Package* surfaces_package()
	{
		static Package* package = Package::static_find_package("TrinexEngine::SurfacePool", true);
		return package;
	}
#endif

	static constexpr uint64_t s_surface_live_threshold = 60 * 3;

	union SurfaceID
	{
		Identifier id;

		struct Value {
			ColorFormat format;
			uint16_t x;
			uint16_t y;
		} value;

		static_assert(sizeof(Value) == sizeof(Identifier));
	};

	static inline Identifier static_calculate_surface_id(ColorFormat format, Vector2u size)
	{
		SurfaceID id;
		id.value.format = format;
		id.value.x      = size.x;
		id.value.y      = size.y;
		return id.id;
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

		return *this;
	}

	RenderSurface* RenderSurfacePool::request_surface(ColorFormat format, Vector2u size)
	{
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

	RenderSurface* RenderSurfacePool::request_transient_surface(ColorFormat format, Vector2u size)
	{
		auto surface = request_surface(format, size);
		logic_thread()->call([object = Pointer(surface), this]() { return_surface(object.ptr()); });
		return surface;
	}

	RenderSurfacePool& RenderSurfacePool::return_surface(RenderSurface* surface)
	{
		auto& pool    = m_pools[static_calculate_surface_id(surface->format(), surface->size())];
		auto& entry   = pool.emplace_back();
		entry.surface = surface;
		entry.frame   = s_surface_live_threshold;
		return *this;
	}
}// namespace Engine
