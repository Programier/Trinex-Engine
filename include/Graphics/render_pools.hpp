#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/fwd.hpp>
#include <Core/pointer.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class RenderSurface;
	class RHITexture;
	class RHIBuffer;
	class RHIFence;
	class RHITimestamp;
	class RHIPipelineStatistics;

	class ENGINE_EXPORT RHIFencePool final
	{
	private:
		struct FenceEntry {
			uint64_t frame  = 0;
			RHIFence* fence = nullptr;
		};

		Vector<FenceEntry> m_pool;
		Vector<RHIFence*> m_transient_fences;

	private:
		RHIFencePool& flush_transient();

	public:
		static RHIFencePool* global_instance();

		RHIFencePool& update();
		RHIFence* request_fence();
		RHIFence* request_transient_fence();
		RHIFencePool& release_all();
		RHIFencePool& return_fence(RHIFence* fence);
	};

	class ENGINE_EXPORT RHIBufferPool final
	{
	private:
		struct BufferEntry {
			uint64_t frame    = 0;
			RHIBuffer* buffer = nullptr;
		};

		Map<Identifier, Vector<BufferEntry>> m_pools;
		Map<RHIBuffer*, Identifier> m_buffer_id;
		Vector<RHIBuffer*> m_transient_buffers;

	private:
		RHIBufferPool& flush_transient();

	public:
		static RHIBufferPool* global_instance();

		RHIBufferPool& update();
		RHIBuffer* request_buffer(uint32_t size, RHIBufferCreateFlags flags);
		RHIBuffer* request_transient_buffer(uint32_t size, RHIBufferCreateFlags flags);
		RHIBufferPool& release_all();
		RHIBufferPool& return_buffer(RHIBuffer* buffer);
	};

	class ENGINE_EXPORT RHISurfacePool final
	{
	private:
		struct SurfaceEntry {
			uint64_t frame      = 0;
			RHITexture* surface = nullptr;
		};

		Map<Identifier, Vector<SurfaceEntry>> m_pools;
		Map<RHITexture*, Identifier> m_surface_id;
		Vector<RHITexture*> m_transient_textures;

	private:
		RHISurfacePool& flush_transient();

	public:
		static RHISurfacePool* global_instance();

		RHISurfacePool& update();
		RHITexture* request_surface(struct RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags = {});
		RHITexture* request_transient_surface(struct RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags = {});
		RHISurfacePool& release_all();
		RHISurfacePool& return_surface(RHITexture* surface);
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

	class ENGINE_EXPORT RHIPipelineStatisticsPool final
	{
	private:
		struct StatsEntry {
			uint64_t frame               = 0;
			RHIPipelineStatistics* stats = nullptr;
		};

		Vector<StatsEntry> m_pool;

	public:
		static RHIPipelineStatisticsPool* global_instance();

		RHIPipelineStatisticsPool& update();
		RHIPipelineStatistics* request_statistics();
		RHIPipelineStatisticsPool& release_all();
		RHIPipelineStatisticsPool& return_statistics(RHIPipelineStatistics* stats);
	};
}// namespace Engine
