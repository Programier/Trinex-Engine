#pragma once
#include <Core/engine_types.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
	struct RHI_Object;
	struct VulkanDescriptorSetManager;

	struct VulkanCommandBuffer final {
	private:
		enum class State
		{
			IsReadyForBegin,
			IsInsideBegin,
			IsInsideRenderPass,
			HasEnded,
			Submitted,
		};

		Vector<RHI_Object*> m_references;
		std::vector<vk::Semaphore> m_wait_semaphores;
		std::vector<vk::PipelineStageFlags> m_wait_flags;
		VulkanDescriptorSetManager* m_descriptor_set_manager = nullptr;

		State m_state = State::IsReadyForBegin;

		VulkanCommandBuffer(struct VulkanCommandBufferPool* pool);
		VulkanCommandBuffer& destroy(struct VulkanCommandBufferPool* pool);
		~VulkanCommandBuffer();

	public:
		vk::CommandBuffer m_cmd;
		struct VulkanFence* m_fence = nullptr;

		VulkanCommandBuffer& add_object(RHI_Object* object);
		VulkanCommandBuffer& release_references();

		VulkanCommandBuffer& refresh_fence_status();
		VulkanCommandBuffer& begin();
		VulkanCommandBuffer& end();
		VulkanCommandBuffer& begin_render_pass(struct VulkanRenderTargetBase* rt);
		VulkanCommandBuffer& end_render_pass();
		VulkanCommandBuffer& add_wait_semaphore(vk::PipelineStageFlags flags, vk::Semaphore semaphore);
		VulkanCommandBuffer& submit(vk::Semaphore* signal_semaphore = nullptr);
		VulkanCommandBuffer& wait();

		inline bool is_ready_for_begin() const
		{
			return m_state == State::IsReadyForBegin;
		}

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

		inline VulkanDescriptorSetManager* descriptor_set_manager()
		{
			return m_descriptor_set_manager;
		}

		friend struct VulkanCommandBufferPool;
		friend struct VulkanQueue;
	};

	struct VulkanCommandBufferPool {
		vk::CommandPool m_pool;

		VulkanCommandBufferPool& refresh_fence_status(const VulkanCommandBuffer* skip_cmd_buffer = nullptr);

	private:
		Vector<VulkanCommandBuffer*> m_cmd_buffers;

		VulkanCommandBuffer* create();

		VulkanCommandBufferPool();
		~VulkanCommandBufferPool();

		friend struct VulkanCommandBufferManager;
	};

	struct VulkanCommandBufferManager {
		VulkanCommandBufferPool m_pool;

	private:
		VulkanCommandBuffer* m_current = nullptr;

		VulkanCommandBufferManager& bind_new_command_buffer();

	public:
		FORCE_INLINE VulkanCommandBuffer* active_command_buffer() const
		{
			return m_current;
		}

		FORCE_INLINE bool has_pending_active_cmd_buffer() const
		{
			return m_current != nullptr;
		}

		FORCE_INLINE VulkanCommandBuffer* command_buffer()
		{
			if (!m_current)
				bind_new_command_buffer();
			return m_current;
		}

		FORCE_INLINE void refresh_fence_status(VulkanCommandBuffer* skip_cmd_buffer = nullptr)
		{
			m_pool.refresh_fence_status(skip_cmd_buffer);
		}

		VulkanCommandBufferManager& submit_active_cmd_buffer(vk::Semaphore* semaphore = nullptr);
	};
}// namespace Engine
