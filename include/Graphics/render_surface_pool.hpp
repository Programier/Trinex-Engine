#pragma once
#include <Core/enums.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/pointer.hpp>
#include <Core/render_resource_ptr.hpp>
#include <Graphics/types/color_format.hpp>

namespace Engine
{
	class RenderSurface;
	struct RHI_Texture2D;
	struct RHI_Buffer;

	class ENGINE_EXPORT RHIBufferPool final
	{
	private:
		struct SurfaceEntry {
			uint64_t frame     = 0;
			RHI_Buffer* buffer = nullptr;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;
		Map<RHI_Buffer*, Identifier> m_buffer_id;

	public:
		static RHIBufferPool* global_instance();

		RHIBufferPool& update(float dt);
		RHI_Buffer* request_buffer(uint32_t size, BufferCreateFlags flags);
		RHI_Buffer* request_transient_buffer(uint32_t size, BufferCreateFlags flags);
		RHIBufferPool& release_all();
		RHIBufferPool& return_buffer(RHI_Buffer* buffer);
	};

	class ENGINE_EXPORT RHISurfacePool final
	{
	private:
		struct SurfaceEntry {
			uint64_t frame         = 0;
			RHI_Texture2D* surface = nullptr;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;
		Map<RHI_Texture2D*, Identifier> m_surface_id;

	public:
		static RHISurfacePool* global_instance();

		RHISurfacePool& update(float dt);
		RHI_Texture2D* request_surface(SurfaceFormat format, Vector2u size);
		RHI_Texture2D* request_transient_surface(SurfaceFormat format, Vector2u size);
		RHISurfacePool& release_all();
		RHISurfacePool& return_surface(RHI_Texture2D* surface);
	};

	class ENGINE_EXPORT RenderSurfacePool final
	{
	private:
		struct SurfaceEntry {
			uint64_t frame = 0;
			Pointer<RenderSurface> surface;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;

	public:
		static RenderSurfacePool* global_instance();

		RenderSurfacePool& update(float dt);
		RenderSurface* request_surface(SurfaceFormat format, Vector2u size);
		RenderSurface* request_transient_surface(SurfaceFormat format, Vector2u size);
		RenderSurfacePool& return_surface(RenderSurface* surface);
	};
}// namespace Engine
