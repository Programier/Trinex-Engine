#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/pointer.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class RenderSurface;
	struct RHI_Texture;
	struct RHI_Buffer;
	struct RHI_Fence;
	struct RHITimestamp;

	class ENGINE_EXPORT RHIFencePool final
	{
	private:
		struct FenceEntry {
			uint64_t frame   = 0;
			RHI_Fence* fence = nullptr;
		};

		Vector<FenceEntry> m_pool;
		Vector<RHI_Fence*> m_transient_fences;

	private:
		RHIFencePool& flush_transient();

	public:
		static RHIFencePool* global_instance();

		RHIFencePool& update();
		RHI_Fence* request_fence();
		RHI_Fence* request_transient_fence();
		RHIFencePool& release_all();
		RHIFencePool& return_fence(RHI_Fence* fence);
	};

	class ENGINE_EXPORT RHIBufferPool final
	{
	private:
		struct BufferEntry {
			uint64_t frame     = 0;
			RHI_Buffer* buffer = nullptr;
		};

		Map<Identifier, Vector<BufferEntry>> m_pools;
		Map<RHI_Buffer*, Identifier> m_buffer_id;
		Vector<RHI_Buffer*> m_transient_buffers;

	private:
		RHIBufferPool& flush_transient();

	public:
		static RHIBufferPool* global_instance();

		RHIBufferPool& update();
		RHI_Buffer* request_buffer(uint32_t size, RHIBufferCreateFlags flags);
		RHI_Buffer* request_transient_buffer(uint32_t size, RHIBufferCreateFlags flags);
		RHIBufferPool& release_all();
		RHIBufferPool& return_buffer(RHI_Buffer* buffer);
	};

	class ENGINE_EXPORT RHISurfacePool final
	{
	private:
		struct SurfaceEntry {
			uint64_t frame       = 0;
			RHI_Texture* surface = nullptr;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;
		Map<RHI_Texture*, Identifier> m_surface_id;
		Vector<RHI_Texture*> m_transient_textures;

	private:
		RHISurfacePool& flush_transient();

	public:
		static RHISurfacePool* global_instance();

		RHISurfacePool& update();
		RHI_Texture* request_surface(struct RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags = {});
		RHI_Texture* request_transient_surface(struct RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags = {});
		RHISurfacePool& release_all();
		RHISurfacePool& return_surface(RHI_Texture* surface);
	};

	class ENGINE_EXPORT RenderSurfacePool final
	{
	private:
		struct SurfaceEntry {
			uint64_t frame = 0;
			Pointer<RenderSurface> surface;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;
		Vector<RenderSurface*> m_transient_surfaces;

	private:
		RenderSurfacePool& flush_transient();

	public:
		static RenderSurfacePool* global_instance();

		RenderSurfacePool& update();
		RenderSurface* request_surface(struct RHISurfaceFormat format, Vector2u size);
		RenderSurface* request_transient_surface(struct RHISurfaceFormat format, Vector2u size);
		RenderSurfacePool& return_surface(RenderSurface* surface);
	};

	class ENGINE_EXPORT RHITimestampPool final
	{
	private:
		struct TimestampEntry {
			uint64_t frame          = 0;
			RHITimestamp* timestamp = nullptr;
		};

		Vector<TimestampEntry> m_pool;
		
	public:
		static RHITimestampPool* global_instance();

		RHITimestampPool& update();
		RHITimestamp* request_timestamp();
		RHITimestampPool& release_all();
		RHITimestampPool& return_timestamp(RHITimestamp* timestamp);
	};
}// namespace Engine
