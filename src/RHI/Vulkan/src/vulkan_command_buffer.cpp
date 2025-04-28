#include <Core/exception.hpp>
#include <Core/profiler.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
	VulkanCommandBuffer::VulkanCommandBuffer(struct VulkanCommandBufferPool* pool)
	{
		vk::CommandBufferAllocateInfo alloc_info(pool->m_pool, vk::CommandBufferLevel::ePrimary, 1);
		m_cmd   = API->m_device.allocateCommandBuffers(alloc_info).front();
		m_fence = VulkanFence::create(false);

		m_descriptor_set_manager = new VulkanDescriptorSetManager();
		m_uniform_buffer         = new VulkanUniformBufferManager();
	}

	VulkanCommandBuffer& VulkanCommandBuffer::add_object(RHI_Object* object)
	{
		if (object)
		{
			m_references.push_back(object);
			object->add_reference();
		}
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::release_references()
	{
		for (RHI_Object* object : m_references)
		{
			object->release();
		}
		m_references.clear();
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::refresh_fence_status()
	{
		if (m_state == State::Submitted)
		{
			if (m_fence->is_signaled())
			{
				m_state = State::IsReadyForBegin;
				m_cmd.reset();
				m_fence->reset();
				m_uniform_buffer->reset();
				release_references();
			}
		}

		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::begin()
	{
		trinex_check(m_state == State::IsReadyForBegin, "Vulkan cmd state must be ready for begin");
		m_cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		m_state = State::IsInsideBegin;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::end()
	{
		trinex_check(is_outside_render_pass(), "Command Buffer must be in outside render pass state!");

		m_cmd.end();
		m_state = State::HasEnded;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::begin_render_pass(struct VulkanRenderTargetBase* rt)
	{
		trinex_check(has_begun(), "Command Buffer must be begun!");

		vk::Rect2D area({0, 0}, {static_cast<uint32_t>(rt->m_size.x), static_cast<uint32_t>(rt->m_size.y)});
		vk::RenderPassBeginInfo info(rt->m_render_pass->m_render_pass, rt->m_framebuffer, area);
		m_cmd.beginRenderPass(info, vk::SubpassContents::eInline);

		m_state = State::IsInsideRenderPass;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::end_render_pass()
	{
		trinex_check(is_inside_render_pass(), "Command Buffer must be inside render pass!");
		m_cmd.endRenderPass();

		m_state = State::IsInsideBegin;
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::add_wait_semaphore(vk::PipelineStageFlags flags, vk::Semaphore semaphore)
	{
		m_wait_semaphores.push_back(semaphore);
		m_wait_flags.push_back(flags);
		return *this;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::submit(vk::Semaphore* signal_semaphore)
	{
		trinex_profile_cpu_n("VulkanCommandBuffer::submit");
		trinex_check(has_ended(), "Command Buffer must be in ended state!");

		API->m_graphics_queue->submit(this, signal_semaphore ? 1 : 0, signal_semaphore);
		m_descriptor_set_manager->submit();
		m_wait_flags.clear();
		m_wait_semaphores.clear();
		m_state = State::Submitted;
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

	VulkanCommandBuffer& VulkanCommandBuffer::destroy(struct VulkanCommandBufferPool* pool)
	{
		release_references();
		m_fence->release();
		API->m_device.freeCommandBuffers(pool->m_pool, m_cmd);
		return *this;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		delete m_descriptor_set_manager;
		delete m_uniform_buffer;
	}

	VulkanCommandBufferPool::VulkanCommandBufferPool()
	{
		m_pool = API->m_device.createCommandPool(
		        vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, API->m_graphics_queue->m_index));
	}

	VulkanCommandBuffer* VulkanCommandBufferPool::create()
	{
		auto buffer = new VulkanCommandBuffer(this);
		m_cmd_buffers.push_back(buffer);
		info_log("Vulkan", "Allocate new command buffer. Total count: %zu", m_cmd_buffers.size());
		return buffer;
	}

	VulkanCommandBufferPool::~VulkanCommandBufferPool()
	{
		for (auto buffer : m_cmd_buffers)
		{
			buffer->destroy(this);
			delete buffer;
		}
		API->m_device.destroyCommandPool(m_pool);
	}

	VulkanCommandBufferPool& VulkanCommandBufferPool::refresh_fence_status(const VulkanCommandBuffer* skip_cmd_buffer)
	{
		for (auto buffer : m_cmd_buffers)
		{
			if (buffer != skip_cmd_buffer)
			{
				buffer->refresh_fence_status();
			}
		}

		return *this;
	}

	VulkanCommandBufferManager& VulkanCommandBufferManager::bind_new_command_buffer()
	{
		for (auto buffer : m_pool.m_cmd_buffers)
		{
			buffer->refresh_fence_status();

			if (buffer->is_ready_for_begin())
			{
				m_current = buffer;
				break;
			}
			else
			{
				trinex_check(buffer->is_submitted(), "Buffer is not submitted and fence status is not signaled");
			}
		}

		if (m_current == nullptr)
		{
			m_current = m_pool.create();
		}

		m_current->begin();
		return *this;
	}

	VulkanCommandBufferManager& VulkanCommandBufferManager::submit_active_cmd_buffer(vk::Semaphore* semaphore)
	{
		if (!m_current->is_submitted() && m_current->has_begun())
		{
			if (m_current->is_inside_render_pass())
			{
				API->end_render_pass();
			}

			m_current->end();
			m_current->submit(semaphore);
		}
		m_current = nullptr;
		return *this;
	}

	VulkanCommandBuffer* VulkanAPI::current_command_buffer()
	{
		return m_cmd_manager->command_buffer();
	}

	vk::CommandBuffer& VulkanAPI::current_command_buffer_handle()
	{
		return m_cmd_manager->command_buffer()->m_cmd;
	}

	VulkanUniformBufferManager* VulkanAPI::uniform_buffer_manager()
	{
		return current_command_buffer()->uniform_buffer_manager();
	}
}// namespace Engine
