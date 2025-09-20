#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
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

		struct Buffer {
			vk::Buffer buffer;
			size_t size;
			size_t offset;

			Buffer(vk::Buffer buffer = {}, size_t size = 0, size_t offset = 0) : buffer(buffer), size(size), offset(offset) {}

			inline bool operator==(const Buffer& other) const
			{
				return buffer == other.buffer && size == other.size && offset == other.offset;
			}

			inline bool operator!=(const Buffer& other) const { return !((*this) == other); }
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

		VulkanUniformBuffer* m_uniform_buffer_pool      = nullptr;
		VulkanUniformBuffer** m_uniform_buffer_push_ptr = nullptr;

		Vector<VulkanUniformBuffer*> m_global_uniform_buffers;

		class VulkanRenderPass* m_render_pass     = nullptr;
		class VulkanRenderTarget* m_render_target = nullptr;

		VulkanPipeline* m_pipeline = nullptr;

		vk::PrimitiveTopology m_primitive_topology = vk::PrimitiveTopology::eTriangleList;
		vk::PolygonMode m_polygon_mode             = vk::PolygonMode::eFill;
		vk::CullModeFlags m_cull_mode              = vk::CullModeFlagBits::eNone;
		vk::FrontFace m_front_face                 = vk::FrontFace::eClockwise;
		vk::ColorComponentFlags m_write_mask       = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		                                       vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	private:
		VulkanUniformBuffer* request_uniform_buffer();
		VulkanStateManager& return_uniform_buffer(VulkanUniformBuffer*);

		VulkanStateManager& flush_state();

	public:
		VulkanResourceState<Buffer> uniform_buffers;
		VulkanResourceState<Buffer> storage_buffers;
		VulkanResourceState<Buffer> uniform_texel_buffers;
		VulkanResourceState<Buffer> storage_texel_buffers;
		VulkanResourceState<vk::Sampler> samplers;
		VulkanResourceState<VulkanTextureSRV*> srv_images;
		VulkanResourceState<VulkanTextureUAV*> uav_images;
		VulkanResourceState<VertexStream> vertex_streams;
		VulkanResourceState<VertexAttribute> vertex_attributes[9];

		VulkanStateManager();
		~VulkanStateManager();

		VulkanStateManager& update_scalar(const void* data, size_t size, size_t offset, byte buffer_index);

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

		VulkanStateManager& bind(vk::PrimitiveTopology topology)
		{
			if (m_primitive_topology != topology)
			{
				m_primitive_topology = topology;
				m_dirty_flags |= PrimitiveTopology;
			}
			return *this;
		}

		VulkanStateManager& bind(vk::PolygonMode mode)
		{
			if (m_polygon_mode != mode)
			{
				m_polygon_mode = mode;
				m_dirty_flags |= PolygonMode;
			}
			return *this;
		}

		VulkanStateManager& bind(vk::CullModeFlags mode)
		{
			if (m_cull_mode != mode)
			{
				m_cull_mode = mode;
				m_dirty_flags |= CullMode;
			}
			return *this;
		}

		VulkanStateManager& bind(vk::FrontFace face)
		{
			if (m_front_face != face)
			{
				m_front_face = face;
				m_dirty_flags |= FrontFace;
			}
			return *this;
		}

		VulkanStateManager& bind(vk::ColorComponentFlags write_mask)
		{
			if (m_write_mask != write_mask)
			{
				m_write_mask = write_mask;
				m_dirty_flags |= WriteMask;
			}
			return *this;
		}

		VulkanCommandHandle* begin_render_pass();
		VulkanCommandHandle* end_render_pass();
		VulkanCommandHandle* flush_graphics();
		VulkanCommandHandle* flush_compute();
		VulkanStateManager& submit();

		vk::PipelineVertexInputStateCreateInfo create_vertex_input(VulkanVertexAttribute* attributes, size_t count);
		Identifier graphics_pipeline_id(VulkanVertexAttribute* attributes, size_t count) const;
		Identifier mesh_pipeline_id() const;

		inline uint64_t dirty_flags() const { return m_dirty_flags; }
		inline bool is_dirty(uint64_t flags) const { return (m_dirty_flags & flags); }
		inline VulkanPipeline* pipeline() const { return m_pipeline; }
		inline VulkanRenderTarget* render_target() const { return m_render_target; }
		inline vk::PrimitiveTopology primitive_topology() const { return m_primitive_topology; }
		inline vk::PolygonMode polygon_mode() const { return m_polygon_mode; }
		inline vk::CullModeFlags cull_mode() const { return m_cull_mode; }
		inline vk::FrontFace front_face() const { return m_front_face; }
		inline vk::ColorComponentFlags write_mask() const { return m_write_mask; }

		friend class VulkanUniformBuffer;
	};
}// namespace Engine
