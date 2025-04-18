#pragma once
#include <Core/structures.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
	enum class VulkanViewportMode
	{
		Undefined = 0,
		Normal    = 1,
		Flipped   = 2
	};

	struct VulkanState {
		struct VulkanRenderPass* m_render_pass              = nullptr;
		struct VulkanRenderTargetBase* m_render_target      = nullptr;
		struct VulkanRenderTargetBase* m_next_render_target = nullptr;

		struct VulkanPipeline* m_pipeline                    = nullptr;
		vk::Pipeline m_vk_pipeline                           = {};
		struct RHI_VertexBuffer* m_current_vertex_buffer[15] = {};
		struct RHI_IndexBuffer* m_current_index_buffer       = nullptr;
		VulkanViewportMode m_viewport_mode                   = VulkanViewportMode::Undefined;
		ViewPort m_viewport;
		Scissor m_scissor;

		inline void reset()
		{
			new (this) VulkanState();
			m_viewport.max_depth = 1.f;
		}

		inline VulkanRenderTargetBase* render_target() { return m_next_render_target ? m_next_render_target : m_render_target; }
	};
}// namespace Engine
