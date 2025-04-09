#pragma once
#include <Core/enums.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/pointer.hpp>
#include <Core/tickable.hpp>

namespace Engine
{
	class RenderSurface;

	class ENGINE_EXPORT RenderSurfacePool : public TickableObject
	{
	private:
		struct SurfaceEntry {
			uint64_t frame = 0;
			Pointer<RenderSurface> surface;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;

	public:
		static RenderSurfacePool* global_instance();

		RenderSurfacePool& update(float dt) override;
		RenderSurface* request_surface(ColorFormat format, Vector2u size);
		RenderSurface* request_transient_surface(ColorFormat format, Vector2u size);
		RenderSurfacePool& return_surface(RenderSurface* surface);
	};
}// namespace Engine
