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
	class RHIContext;

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

	class ENGINE_EXPORT RHITexturePool final
	{
	private:
		struct Key {
			RHISurfaceFormat format;
			RHITextureType type;
			RHITextureCreateFlags flags;

			uint16_t width;
			uint16_t height;
			uint16_t depth;

			inline bool operator==(const Key& key) const = default;
		};

		struct ENGINE_EXPORT Hasher {
			uint64_t operator()(const Key& key) const;
		};

		struct SurfaceEntry {
			uint64_t frame      = 0;
			RHITexture* surface = nullptr;
		};

		Map<Key, Vector<SurfaceEntry>, Hasher> m_pools;
		Map<RHITexture*, Key> m_surface_id;
		Vector<RHITexture*> m_transient_textures;

	private:
		RHITexturePool& flush_transient();

	public:
		static RHITexturePool* global_instance();

		RHITexturePool& update();
		RHITexture* request_surface(RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags = {});
		RHITexture* request_surface(RHITextureType type, RHISurfaceFormat format, Vector3u size,
		                            RHITextureCreateFlags flags = {});
		RHITexture* request_transient_surface(RHISurfaceFormat format, Vector2u size, RHITextureCreateFlags flags = {});
		RHITexture* request_transient_surface(RHITextureType type, RHISurfaceFormat format, Vector3u size,
		                                      RHITextureCreateFlags flags = {});
		RHITexturePool& release_all();
		RHITexturePool& return_surface(RHITexture* surface);
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

	class ENGINE_EXPORT RHIContextPool final
	{
	private:
		struct ContextEntry {
			uint64_t frame      = 0;
			RHIContext* context = nullptr;
		};

		Vector<ContextEntry> m_pool;
		Vector<RHIContext*> m_transient;

	public:
		static RHIContextPool* global_instance();

		RHIContextPool& update();
		RHIContext* request_context();
		RHIContextPool& release_all();
		RHIContextPool& return_context(RHIContext* context);

		RHIContext* begin_context();
		RHIContextPool& end_context(RHIContext* context);
	};
}// namespace Engine
