#pragma once
#include <Core/engine_types.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
	struct RHI_Object;

	struct VulkanCommandBuffer final {
		enum class State
		{
			IsInsideBegin,
			IsInsideRenderPass,
			HasEnded,
			Submitted,
		};

		Vector<RHI_Object*> m_references;
		vk::CommandBuffer m_cmd;
		vk::Fence m_fence;

		State m_state = State::Submitted;

		VulkanCommandBuffer();
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&);

		VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&);

		VulkanCommandBuffer& add_object(RHI_Object* object);
		VulkanCommandBuffer& release_references();

		VulkanCommandBuffer& begin();
		VulkanCommandBuffer& end();
		VulkanCommandBuffer& begin_render_pass(struct VulkanRenderTargetBase* rt);
		VulkanCommandBuffer& end_render_pass();
		VulkanCommandBuffer& submit(const vk::SubmitInfo& info);

		inline bool is_inside_render_pass() const
		{
			return m_state == State::IsInsideRenderPass;
		}

		inline bool is_outside_render_pass() const
		{
			return m_state == State::IsInsideBegin;
		}

		inline bool has_begun() const
		{
			return m_state == State::IsInsideBegin || m_state == State::IsInsideRenderPass;
		}

		inline bool has_ended() const
		{
			return m_state == State::HasEnded;
		}

		inline bool is_submitted() const
		{
			return m_state == State::Submitted;
		}

		~VulkanCommandBuffer();
	};
}// namespace Engine
