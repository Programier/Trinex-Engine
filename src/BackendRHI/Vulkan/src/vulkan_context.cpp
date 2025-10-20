#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_query.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_state.hpp>


namespace Engine
{
	VulkanUniformBuffer* VulkanCommandHandle::UniformBuffer::request_uniform_page(size_t size)
	{
		while (*m_uniform_buffer_current)
		{
			VulkanUniformBuffer* buffer = *m_uniform_buffer_current;
			if (buffer->contains(size))
				return buffer;
			m_uniform_buffer_current = &buffer->next;
		}


		VulkanUniformBuffer* buffer = trx_new VulkanUniformBuffer(size);
		(*m_uniform_buffer_current) = buffer;
		return buffer;
	}

	VulkanCommandHandle::UniformBuffer& VulkanCommandHandle::UniformBuffer::reset()
	{
		VulkanUniformBuffer* buffer = m_uniform_buffer_head;
		m_uniform_buffer_current    = &m_uniform_buffer_head;
		while (buffer)
		{
			buffer->reset();
			buffer = buffer->next;
		}
		return *this;
	}

	VulkanCommandHandle::UniformBuffer& VulkanCommandHandle::UniformBuffer::flush()
	{
		if (*m_uniform_buffer_current)
		{
			(*m_uniform_buffer_current)->flush();
		}

		return *this;
	}

	VulkanCommandHandle::VulkanCommandHandle(VulkanCommandBufferManager* manager)
	{
		m_fence = trx_new VulkanFence(false);

		vk::CommandPool pool = manager->command_pool();
		vk::CommandBufferAllocateInfo alloc_info(pool, vk::CommandBufferLevel::ePrimary, 1);
		static_cast<vk::CommandBuffer&>(*this) = API->m_device.allocateCommandBuffers(alloc_info).front();
	}

	VulkanCommandHandle& VulkanCommandHandle::refresh_fence_status()
	{
		if (m_state == State::Submitted)
		{
			if (m_fence->is_signaled())
			{
				m_state = State::IsReadyForBegin;
				reset();
				m_fence->reset();

				for (auto& page : m_uniform_buffers)
				{
					page.reset();
				}

				for (RHIObject* stagging : m_stagging)
				{
					stagging->destroy();
				}

				m_stagging.clear();
			}
		}

		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::begin()
	{
		trinex_check(m_state == State::IsReadyForBegin, "Vulkan cmd state must be ready for begin");
		vk::CommandBuffer::begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		m_state = State::IsInsideBegin;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::end()
	{
		trinex_check(is_outside_render_pass(), "Command Buffer must be in outside render pass state!");
		vk::CommandBuffer::end();
		m_state = State::HasEnded;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::begin_render_pass(VulkanRenderTarget* rt)
	{
		trinex_check(has_begun(), "Command Buffer must be begun!");

		vk::Rect2D area({0, 0}, {rt->width(), rt->height()});
		vk::RenderPassBeginInfo info(rt->m_render_pass->render_pass(), rt->framebuffer(), area);
		vk::CommandBuffer::beginRenderPass(info, vk::SubpassContents::eInline);

		m_state = State::IsInsideRenderPass;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::end_render_pass()
	{
		trinex_check(is_inside_render_pass(), "Command Buffer must be inside render pass!");
		vk::CommandBuffer::endRenderPass();

		m_state = State::IsInsideBegin;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::enqueue()
	{
		trinex_profile_cpu_n("VulkanCommandBuffer::submit");
		trinex_check(has_ended(), "Command Buffer must be in ended state!");
		API->m_graphics_queue->submit({}, m_fence->fence());
		m_fence->make_pending();
		m_state = State::Submitted;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::wait()
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

	VulkanCommandHandle& VulkanCommandHandle::flush_uniforms()
	{
		for (auto& page : m_uniform_buffers)
		{
			page.flush();
		}
		return *this;
	}

	void VulkanCommandHandle::destroy()
	{
		API->command_buffer_mananger()->return_handle(this);
	}

	VulkanCommandHandle::~VulkanCommandHandle()
	{
		wait();

		auto pool = API->command_buffer_mananger()->command_pool();
		API->m_device.freeCommandBuffers(pool, *this);

		for (auto& page : m_uniform_buffers)
		{
			while (page.m_uniform_buffer_head)
			{
				auto current               = page.m_uniform_buffer_head;
				page.m_uniform_buffer_head = page.m_uniform_buffer_head->next;
				trx_delete_inline(current);
			}
		}
		trx_delete m_fence;
	}

	VulkanCommandBufferManager::VulkanCommandBufferManager()
	{
		m_pool = API->m_device.createCommandPool(
		        vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, API->m_graphics_queue->index()));
	}

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		for (VulkanCommandHandle* handle : m_handles)
		{
			trx_delete handle;
		}

		API->m_device.destroyCommandPool(m_pool);
	}

	VulkanCommandHandle* VulkanCommandBufferManager::request_handle()
	{
		if (!m_handles.empty())
		{
			auto front = m_handles.front();

			if (front->refresh_fence_status().is_ready_for_begin())
			{
				m_handles.pop_front();
				return front;
			}
		}

		return trx_new VulkanCommandHandle(this);
	}

	VulkanCommandBufferManager& VulkanCommandBufferManager::return_handle(VulkanCommandHandle* handle)
	{
		m_handles.push_back(handle);
		handle->add_reference();
		handle->enqueue();
		return *this;
	}

	VulkanContext::VulkanContext() : m_state_manager(trx_new VulkanStateManager()) {}

	VulkanContext::~VulkanContext()
	{
		if (auto handle = end())
		{
			handle->release();
		}
		trx_delete m_state_manager;
	}

	VulkanContext& VulkanContext::begin()
	{
		if (m_cmd == nullptr)
		{
			m_cmd = API->command_buffer_mananger()->request_handle();
			m_cmd->begin();

			reset_state();
		}
		return *this;
	}

	VulkanCommandHandle* VulkanContext::end()
	{
		if (m_cmd == nullptr)
			return nullptr;

		VulkanCommandHandle* cmd = m_cmd;
		m_cmd                    = nullptr;

		if (cmd->is_inside_render_pass())
			cmd->end_render_pass();
		cmd->end();

		m_state_manager->reset();
		return cmd;
	}

	VulkanContext& VulkanContext::execute(RHICommandHandle* handle)
	{
		m_cmd->executeCommands(*static_cast<VulkanCommandHandle*>(handle));
		return *this;
	}

	VulkanContext& VulkanContext::viewport(const RHIViewport& viewport)
	{
		vk::Viewport vulkan_viewport;
		vulkan_viewport.setWidth(viewport.size.x);
		vulkan_viewport.setHeight(viewport.size.y);
		vulkan_viewport.setX(viewport.pos.x);
		vulkan_viewport.setY(viewport.pos.y);
		vulkan_viewport.setMinDepth(viewport.min_depth);
		vulkan_viewport.setMaxDepth(viewport.max_depth);
		m_cmd->setViewport(0, vulkan_viewport);
		return *this;
	}

	VulkanContext& VulkanContext::scissor(const RHIScissor& scissor)
	{
		vk::Rect2D vulkan_scissor;
		vulkan_scissor.offset.setX(scissor.pos.x);
		vulkan_scissor.offset.setY(scissor.pos.y);
		vulkan_scissor.extent.setWidth(scissor.size.x);
		vulkan_scissor.extent.setHeight(scissor.size.y);
		m_cmd->setScissor(0, vulkan_scissor);
		return *this;
	}

	VulkanContext& VulkanContext::draw(size_t vertex_count, size_t vertices_offset)
	{
		m_state_manager->flush_graphics(this)->draw(vertex_count, 1, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
	{
		m_state_manager->flush_graphics(this)->drawIndexed(indices_count, 1, indices_offset, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)
	{
		m_state_manager->flush_graphics(this)->draw(vertex_count, instances, vertex_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
	                                                     size_t instances)
	{
		m_state_manager->flush_graphics(this)->drawIndexed(indices_count, instances, indices_offset, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_mesh(uint32_t x, uint32_t y, uint32_t z)
	{
		m_state_manager->flush_graphics(this)->drawMeshTasksEXT(x, y, z, API->pfn);
		return *this;
	}

	VulkanContext& VulkanContext::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		m_state_manager->flush_compute(this)->dispatch(group_x, group_y, group_z);
		return *this;
	}

	VulkanContext& VulkanContext::push_debug_stage(const char* stage)
	{
		if (API->pfn.vkCmdBeginDebugUtilsLabelEXT)
		{
			VkDebugUtilsLabelEXT label_info = {};
			label_info.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			label_info.pLabelName           = stage;
			label_info.color[0]             = 1.f;
			label_info.color[1]             = 1.f;
			label_info.color[2]             = 1.f;
			label_info.color[3]             = 1.f;

			API->pfn.vkCmdBeginDebugUtilsLabelEXT(*m_cmd, &label_info);
		}
		return *this;
	}

	VulkanContext& VulkanContext::pop_debug_stage()
	{
		if (API->pfn.vkCmdEndDebugUtilsLabelEXT)
		{
			API->pfn.vkCmdEndDebugUtilsLabelEXT(*m_cmd);
		}
		return *this;
	}

	VulkanCommandHandle* VulkanContext::begin_render_pass()
	{
		return m_state_manager->begin_render_pass(this);
	}

	VulkanCommandHandle* VulkanContext::end_render_pass()
	{
		return m_state_manager->end_render_pass(this);
	}

	void VulkanContext::destroy()
	{
		trx_delete this;
	}

	RHIContext* VulkanAPI::create_context()
	{
		return trx_new VulkanContext();
	}
}// namespace Engine
