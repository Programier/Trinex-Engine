#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/structures.hpp>
#include <cstring>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanCommandHandle;
	class VulkanSampler;
	class VulkanBuffer;
	class VulkanTexture;
	class VulkanTextureSRV;
	class VulkanTextureUAV;
	class VulkanUniformBuffer;
	class VulkanPipeline;
	class VulkanContext;
	struct VulkanVertexAttribute;

	template<typename T>
	class VulkanResourceState
	{
	private:
		Vector<T> m_resources;
		Vector<uint8_t> m_dirty_flags;

		void resize(size_t index)
		{
			size_t size = index + 1;
			if (size > m_resources.size())
			{
				m_resources.resize(size);
				m_dirty_flags.resize((size >> 3) + 1, 0);
			}
		}

	public:
		T resource(size_t index) const { return m_resources[index]; }
		bool is_dirty(size_t index) const { return m_dirty_flags[index >> 3] & (1 << (index % 8)); }
		size_t size() const { return m_resources.size(); }

		template<typename Resource>
		void bind(Resource&& resource, size_t index)
		{
			resize(index);
			if (m_resources[index] != resource)
			{
				m_resources[index] = std::forward<Resource>(resource);
				make_dirty(index);
			}
		}

		void flush() { std::memset(m_dirty_flags.data(), 0, m_dirty_flags.size()); }
		void make_dirty() { std::memset(m_dirty_flags.data(), 255, m_dirty_flags.size()); }
		void make_dirty(size_t index) { m_dirty_flags[index >> 3] |= (1 << (index % 8)); }
	};

	class VulkanStateManager
	{
	public:
		enum DirtyFlags
		{
			RenderTarget      = 1 << 0,
			Pipeline          = 1 << 1,
			PrimitiveTopology = 1 << 2,
			PolygonMode       = 1 << 3,
			CullMode          = 1 << 4,
			FrontFace         = 1 << 5,
			WriteMask         = 1 << 6,

			GraphicsMask = RenderTarget | Pipeline | PrimitiveTopology | PolygonMode | CullMode | FrontFace | WriteMask,
			ComputeMask  = Pipeline,
		};

		struct CombinedImage {
			VulkanTextureSRV* srv;
			vk::Sampler sampler;
		};

		struct UniformBuffer {
			vk::Buffer buffer;
			uint32_t size;
			uint32_t offset;

			UniformBuffer(vk::Buffer buffer = {}, uint32_t size = 0, uint32_t offset = 0)
			    : buffer(buffer), size(size), offset(offset)
			{}

			inline bool operator==(const UniformBuffer& other) const { return buffer == other.buffer && offset == other.offset; }
			inline bool operator!=(const UniformBuffer& other) const { return !((*this) == other); }
		};

		struct VertexAttribute {
			uint16_t stream;
			uint16_t offset;

			inline bool operator==(const VertexAttribute& other) const
			{
				return stream == other.stream && offset == other.offset;
			}

			inline bool operator!=(const VertexAttribute& other) const { return !((*this) == other); }
		};

		struct VertexStream {
			vk::VertexInputRate rate;
			uint16_t stride;

			inline bool operator==(const VertexStream& other) const { return stride == other.stride && rate == other.rate; }
			inline bool operator!=(const VertexStream& other) const { return !((*this) == other); }
		};

	private:
		uint64_t m_dirty_flags;

		class VulkanRenderPass* m_render_pass     = nullptr;
		class VulkanRenderTarget* m_render_target = nullptr;

		VulkanPipeline* m_pipeline = nullptr;

		struct GraphicsState {
			RHIPrimitiveTopology topology;
			RHIPolygonMode polygon_mode;
			RHICullMode cull_mode;
			RHIFrontFace front_face;
			RHIColorComponent write_mask;

			void init();
		} m_graphics_state;


	private:
		VulkanStateManager& flush_state(VulkanCommandHandle* handle);

	public:
		VulkanResourceState<UniformBuffer> uniform_buffers;
		VulkanResourceState<vk::Buffer> storage_buffers;
		VulkanResourceState<vk::Buffer> uniform_texel_buffers;
		VulkanResourceState<vk::Buffer> storage_texel_buffers;
		VulkanResourceState<vk::Sampler> samplers;
		VulkanResourceState<vk::AccelerationStructureKHR> acceleration_structures;
		VulkanResourceState<VulkanTextureSRV*> srv_images;
		VulkanResourceState<VulkanTextureUAV*> uav_images;
		VulkanResourceState<VertexStream> vertex_streams;
		VulkanResourceState<VertexAttribute> vertex_attributes[9];

		VulkanStateManager();
		~VulkanStateManager();

		VulkanStateManager& update_scalar(VulkanContext* ctx, const void* data, size_t size, size_t offset, byte buffer_index);

		VulkanStateManager& bind(VulkanPipeline* pipeline)
		{
			if (m_pipeline != pipeline)
			{
				m_pipeline = pipeline;
				m_dirty_flags |= Pipeline;
			}
			return *this;
		}

		VulkanStateManager& bind(VulkanRenderTarget* target)
		{
			if (m_render_target != target)
			{
				m_render_target = target;
				m_dirty_flags |= RenderTarget;
			}

			return *this;
		}

		VulkanStateManager& bind(RHIPrimitiveTopology topology)
		{
			if (m_graphics_state.topology != topology)
			{
				m_graphics_state.topology = topology;
				m_dirty_flags |= PrimitiveTopology;
			}
			return *this;
		}

		VulkanStateManager& bind(RHIPolygonMode mode)
		{
			if (m_graphics_state.polygon_mode != mode)
			{
				m_graphics_state.polygon_mode = mode;
				m_dirty_flags |= PolygonMode;
			}
			return *this;
		}

		VulkanStateManager& bind(RHICullMode mode)
		{
			if (m_graphics_state.cull_mode != mode)
			{
				m_graphics_state.cull_mode = mode;
				m_dirty_flags |= CullMode;
			}
			return *this;
		}

		VulkanStateManager& bind(RHIFrontFace face)
		{
			if (m_graphics_state.front_face != face)
			{
				m_graphics_state.front_face = face;
				m_dirty_flags |= FrontFace;
			}
			return *this;
		}

		VulkanStateManager& bind(RHIColorComponent write_mask)
		{
			if (m_graphics_state.write_mask != write_mask)
			{
				m_graphics_state.write_mask = write_mask;
				m_dirty_flags |= WriteMask;
			}
			return *this;
		}

		VulkanCommandHandle* begin_render_pass(VulkanContext* ctx);
		VulkanCommandHandle* end_render_pass(VulkanContext* ctx);
		VulkanCommandHandle* flush_graphics(VulkanContext* ctx);
		VulkanCommandHandle* flush_compute(VulkanContext* ctx);
		VulkanCommandHandle* flush_raytrace(VulkanContext* ctx);
		VulkanStateManager& reset();

		vk::PipelineVertexInputStateCreateInfo create_vertex_input(VulkanVertexAttribute* attributes, size_t count);
		uint64_t graphics_pipeline_id(VulkanVertexAttribute* attributes, size_t count) const;
		uint64_t mesh_pipeline_id() const;

		inline uint64_t dirty_flags() const { return m_dirty_flags; }
		inline bool is_dirty(uint64_t flags) const { return (m_dirty_flags & flags); }
		inline VulkanPipeline* pipeline() const { return m_pipeline; }
		inline VulkanRenderTarget* render_target() const { return m_render_target; }
		inline RHIPrimitiveTopology primitive_topology() const { return m_graphics_state.topology; }
		inline RHIPolygonMode polygon_mode() const { return m_graphics_state.polygon_mode; }
		inline RHICullMode cull_mode() const { return m_graphics_state.cull_mode; }
		inline RHIFrontFace front_face() const { return m_graphics_state.front_face; }
		inline RHIColorComponent write_mask() const { return m_graphics_state.write_mask; }

		friend class VulkanUniformBuffer;
	};
}// namespace Engine
