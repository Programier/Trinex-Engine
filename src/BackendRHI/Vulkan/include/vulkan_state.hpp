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
	class VulkanRenderTarget;
	class VulkanRenderPass;
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
		enum DirtyFlags : uint32_t
		{
			RenderTarget      = 1 << 0,
			Pipeline          = 1 << 1,
			DepthState        = 1 << 2,
			StencilState      = 1 << 3,
			BlendingState     = 1 << 4,
			PrimitiveTopology = 1 << 5,
			PolygonMode       = 1 << 6,
			CullMode          = 1 << 7,
			FrontFace         = 1 << 8,
			WriteMask         = 1 << 9,
			ShadingRate       = 1 << 10,
			Viewport          = 1 << 11,
			Scissor           = 1 << 12,

			GeneralMask  = ShadingRate | Viewport | Scissor,
			GraphicsMask = RenderTarget | Pipeline | DepthState | StencilState | BlendingState | PrimitiveTopology | PolygonMode |
			               CullMode | FrontFace | WriteMask,
			ComputeMask = Pipeline,
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

			inline bool operator==(const UniformBuffer& other) const
			{
				return buffer == other.buffer && offset == other.offset && size == other.size;
			}

			inline bool operator!=(const UniformBuffer& other) const { return !((*this) == other); }
		};

		struct VertexAttribute {
			uint16_t stream;
			uint16_t offset;
			RHIVertexFormat format;

			inline bool operator==(const VertexAttribute& other) const
			{
				return stream == other.stream && offset == other.offset && format == other.format;
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
		uint32_t m_dirty_flags;

		VulkanRenderPass* m_render_pass     = nullptr;
		VulkanRenderTarget* m_render_target = nullptr;
		VulkanPipeline* m_pipeline          = nullptr;

		struct GraphicsState {
			RHIDepthState depth;
			RHIStencilState stencil;
			RHIBlendingState blending;

			RHIPrimitiveTopology topology;
			RHIPolygonMode polygon_mode;
			RHICullMode cull_mode;
			RHIFrontFace front_face;
			RHIColorComponent write_mask;
			RHIShadingRate rate;
			RHIViewport viewport;
			RHIScissor scissor;

			void init();
		} m_graphics_state;


	private:
		VulkanStateManager& flush_state(VulkanCommandHandle* handle, uint32_t mask);
		VulkanStateManager& update_viewport_and_scissor(VulkanRenderTarget* target);

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
		VulkanResourceState<VertexAttribute> vertex_attributes;

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
				update_viewport_and_scissor(target);
				m_render_target = target;
				m_dirty_flags |= RenderTarget;
			}

			return *this;
		}

		VulkanStateManager& bind(const RHIDepthState& state)
		{
			if (m_graphics_state.depth != state)
			{
				m_graphics_state.depth.func         = state.func;
				m_graphics_state.depth.enable       = state.enable;
				m_graphics_state.depth.write_enable = state.write_enable;
				m_dirty_flags |= DepthState;
			}
			return *this;
		}

		VulkanStateManager& bind(const RHIStencilState& state)
		{
			if (m_graphics_state.stencil != state)
			{
				m_graphics_state.stencil.fail         = state.fail;
				m_graphics_state.stencil.depth_pass   = state.depth_pass;
				m_graphics_state.stencil.depth_fail   = state.depth_fail;
				m_graphics_state.stencil.compare      = state.compare;
				m_graphics_state.stencil.compare_mask = state.compare_mask;
				m_graphics_state.stencil.write_mask   = state.write_mask;
				m_graphics_state.stencil.reference    = state.reference;
				m_graphics_state.stencil.enable       = state.enable;
				m_dirty_flags |= StencilState;
			}
			return *this;
		}

		VulkanStateManager& bind(const RHIBlendingState& state)
		{
			if (m_graphics_state.blending != state)
			{
				m_graphics_state.blending.src_color_func = state.src_color_func;
				m_graphics_state.blending.dst_color_func = state.dst_color_func;
				m_graphics_state.blending.color_op       = state.color_op;
				m_graphics_state.blending.src_alpha_func = state.src_alpha_func;
				m_graphics_state.blending.dst_alpha_func = state.dst_alpha_func;
				m_graphics_state.blending.alpha_op       = state.alpha_op;
				m_graphics_state.blending.enable         = state.enable;
				m_dirty_flags |= BlendingState;
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

		VulkanStateManager& bind(RHIViewport viewport)
		{
			if (m_graphics_state.viewport != viewport)
			{
				m_graphics_state.viewport = viewport;
				m_dirty_flags |= Viewport;
			}
			return *this;
		}

		VulkanStateManager& bind(RHIScissor scissor)
		{
			if (m_graphics_state.scissor != scissor)
			{
				m_graphics_state.scissor = scissor;
				m_dirty_flags |= Scissor;
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
		uint128_t graphics_pipeline_id(VulkanVertexAttribute* attributes, size_t count) const;
		uint128_t mesh_pipeline_id() const;

		inline uint32_t dirty_flags() const { return m_dirty_flags; }
		inline bool is_dirty(uint32_t flags) const { return (m_dirty_flags & flags); }
		inline uint32_t remove_dirty(uint32_t flags) { return (m_dirty_flags &= ~flags); }
		inline VulkanPipeline* pipeline() const { return m_pipeline; }
		inline VulkanRenderTarget* render_target() const { return m_render_target; }
		inline const RHIDepthState& depth_state() const { return m_graphics_state.depth; }
		inline const RHIStencilState& stencil_state() const { return m_graphics_state.stencil; }
		inline const RHIBlendingState& blending_state() const { return m_graphics_state.blending; }
		inline RHIPrimitiveTopology primitive_topology() const { return m_graphics_state.topology; }
		inline RHIPolygonMode polygon_mode() const { return m_graphics_state.polygon_mode; }
		inline RHICullMode cull_mode() const { return m_graphics_state.cull_mode; }
		inline RHIFrontFace front_face() const { return m_graphics_state.front_face; }
		inline RHIColorComponent write_mask() const { return m_graphics_state.write_mask; }

		friend class VulkanUniformBuffer;
	};
}// namespace Engine
