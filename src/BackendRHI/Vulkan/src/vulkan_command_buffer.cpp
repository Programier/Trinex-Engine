#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>

namespace Engine
{
	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBufferManager* manager)
	{
		m_fence = allocate<VulkanFence>(false);

		vk::CommandPool pool = manager->command_pool();
		vk::CommandBufferAllocateInfo alloc_info(pool, vk::CommandBufferLevel::ePrimary, 1);
		static_cast<vk::CommandBuffer&>(*this) = API->m_device.allocateCommandBuffers(alloc_info).front();
	}

	VulkanCommandBuffer& VulkanCommandBuffer::refresh_fence_status()
	{
		if (m_state == State::Submitted)
		{
			if (m_fence->is_signaled())
			{
				m_state = State::IsReadyForBegin;
				reset();
				m_fence->reset();
				++m_fence_signaled_count;

				destroy_objects();
			}
		}

		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::begin()
	{
		trinex_check(m_state == State::IsReadyForBegin, "Vulkan cmd state must be ready for begin");
		vk::CommandBuffer::begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		m_state = State::IsInsideBegin;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::end()
	{
		trinex_check(is_outside_render_pass(), "Command Buffer must be in outside render pass state!");
		vk::CommandBuffer::end();
		m_state = State::HasEnded;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::begin_render_pass(struct VulkanRenderTargetBase* rt)
	{
		trinex_check(has_begun(), "Command Buffer must be begun!");

		vk::Rect2D area({0, 0}, {static_cast<uint32_t>(rt->m_size.x), static_cast<uint32_t>(rt->m_size.y)});
		vk::RenderPassBeginInfo info(rt->m_render_pass->render_pass(), rt->m_framebuffer, area);
		vk::CommandBuffer::beginRenderPass(info, vk::SubpassContents::eInline);

		m_state = State::IsInsideRenderPass;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::end_render_pass()
	{
		trinex_check(is_inside_render_pass(), "Command Buffer must be inside render pass!");
		vk::CommandBuffer::endRenderPass();

		m_state = State::IsInsideBegin;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::add_wait_semaphore(vk::PipelineStageFlags flags, vk::Semaphore semaphore)
	{
		m_wait_semaphores.push_back(semaphore);
		m_wait_flags.push_back(flags);
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::submit(vk::Semaphore semaphore)
	{
		trinex_profile_cpu_n("VulkanCommandBuffer::submit");
		trinex_check(has_ended(), "Command Buffer must be in ended state!");

		if (semaphore)
		{
			API->m_graphics_queue->submit(this, 1, &semaphore);
		}
		else
		{
			API->m_graphics_queue->submit(this);
		}
		m_wait_flags.clear();
		m_wait_semaphores.clear();
		m_state = State::Submitted;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::destroy_object(RHIObject* object)
	{
		m_pending_destroy.push_back(object);
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::wait()
	{
		trinex_profile_cpu_n("VulkanCommandBuffer::wait");
		if (m_state == State::Submitted)
		{
			if (!m_fence->is_signaled())
				m_fence->wait();

			refresh_fence_status();
		}
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::destroy_objects()
	{
		while (!m_pending_destroy.empty())
		{
			RHIObject* object = m_pending_destroy.back();
			m_pending_destroy.pop_back();
			object->destroy();
		}

		return *this;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		auto pool = API->command_buffer_mananger()->command_pool();
		API->m_device.freeCommandBuffers(pool, *this);

		Engine::release(m_fence);
		destroy_objects();
	}

	VulkanCommandBufferManager::VulkanCommandBufferManager()
	{
		m_pool = API->m_device.createCommandPool(
		        vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, API->m_graphics_queue->index()));

		m_current  = create_node();
		m_first    = create_node();
		m_push_ptr = &m_first->next;

		m_current->command_buffer->begin();
	}

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		while (m_first)
		{
			m_first = destroy_node(m_first);
		}

		destroy_node(m_current);
		API->m_device.destroyCommandPool(m_pool);
	}

	VulkanCommandBufferManager::Node* VulkanCommandBufferManager::create_node()
	{
		Node* node           = allocate<Node>();
		node->command_buffer = allocate<VulkanCommandBuffer>(this);
		node->next           = nullptr;
		return node;
	}

	VulkanCommandBufferManager::Node* VulkanCommandBufferManager::destroy_node(Node* node)
	{
		Node* next = node->next;
		release(node->command_buffer);
		release(node);
		return next;
	}

	VulkanCommandBufferManager::Node* VulkanCommandBufferManager::request_node()
	{
		auto buffer = m_first->command_buffer;
		buffer->refresh_fence_status();

		if (buffer->is_ready_for_begin())
		{
			Node* result = m_first;
			m_first      = m_first->next;
			buffer->begin();
			return result;
		}

		Node* node = create_node();
		node->command_buffer->begin();
		return node;
	}

	VulkanCommandBufferManager& VulkanCommandBufferManager::submit(vk::Semaphore semaphore)
	{
		VulkanCommandBuffer* buffer = m_current->command_buffer;

		if (buffer->is_inside_render_pass())
		{
			API->end_render_pass();
		}

		buffer->end();
		buffer->submit(semaphore);

		m_current->next = nullptr;
		(*m_push_ptr)   = m_current;
		m_push_ptr      = &m_current->next;

		m_current = request_node();
		return *this;
	}

	VulkanCommandBuffer* VulkanAPI::current_command_buffer()
	{
		return m_cmd_manager->current();
	}
}// namespace Engine
