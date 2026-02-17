#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_descriptor.hpp>
#include <vulkan_enums.hpp>
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

	VulkanCommandHandle::VulkanCommandHandle(VulkanCommandBufferManager* manager, RHIContextFlags flags) : m_flags(flags)
	{
		m_fence   = trx_new VulkanFence(false);
		m_manager = manager;

		const bool secondary               = is_secondary();
		const vk::CommandBufferLevel level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;

		vk::CommandPool pool = manager->command_pool();
		vk::CommandBufferAllocateInfo alloc_info(pool, level, 1);

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
			}
		}

		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::begin(const vk::CommandBufferBeginInfo& info)
	{
		trinex_check(m_state == State::IsReadyForBegin, "Vulkan cmd state must be ready for begin");

		vk::CommandBuffer::begin(info);

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
		for (RHIObject* stagging : m_stagging)
		{
			stagging->destroy();
		}

		m_stagging.clear();
		m_manager->return_handle(this);
	}

	VulkanCommandHandle::~VulkanCommandHandle()
	{
		wait();

		auto pool = m_manager->command_pool();
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
		for (VulkanCommandHandle* handle : m_primary)
		{
			trx_delete handle;
		}

		for (VulkanCommandHandle* handle : m_secondary)
		{
			trx_delete handle;
		}

		API->m_device.destroyCommandPool(m_pool);
	}

	VulkanCommandBufferManager* VulkanCommandBufferManager::instance()
	{
		static thread_local VulkanCommandBufferManager* manager = trx_new VulkanCommandBufferManager();
		return manager;
	}

	VulkanCommandHandle* VulkanCommandBufferManager::request_handle(RHIContextFlags flags)
	{
		{
			ScopeLock lock(m_critical);

			Deque<VulkanCommandHandle*>* handles = flags & RHIContextFlags::Secondary ? &m_secondary : &m_primary;

			if (!handles->empty())
			{
				auto front = handles->front();

				if (front->refresh_fence_status().is_ready_for_begin())
				{
					handles->pop_front();
					return front;
				}
			}
		}

		return trx_new VulkanCommandHandle(this, flags);
	}

	VulkanCommandBufferManager& VulkanCommandBufferManager::return_handle(VulkanCommandHandle* handle)
	{
		ScopeLock lock(m_critical);

		auto& queue = handle->is_primary() ? m_primary : m_secondary;
		queue.push_back(handle);

		handle->add_reference();
		handle->enqueue();
		return *this;
	}

	VulkanContext::VulkanContext(RHIContextFlags flags) : m_state_manager(trx_new VulkanStateManager()), m_flags(flags) {}

	VulkanContext::~VulkanContext()
	{
		if (auto handle = end())
		{
			handle->release();
		}
		trx_delete m_state_manager;
	}

	VulkanContext& VulkanContext::begin(RHIContext* primary)
	{
		if (m_cmd == nullptr)
		{
			m_cmd                = VulkanCommandBufferManager::instance()->request_handle(m_flags);
			VulkanContext* owner = static_cast<VulkanContext*>(primary);

			if (is_secondary())
			{
				if (owner->handle()->is_outside_render_pass())
					owner->begin_render_pass();

				vk::CommandBufferBeginInfo begin_info;
				vk::CommandBufferInheritanceInfo inherit;

				inherit.setFramebuffer(owner->state()->render_target()->framebuffer());
				inherit.setRenderPass(owner->state()->render_target()->render_pass()->render_pass());
				
				begin_info.setPInheritanceInfo(&inherit);
				begin_info.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue);

				m_cmd->begin(begin_info);
				m_state_manager->copy(owner->m_state_manager);
				copy_state(primary);
			}
			else
			{
				m_cmd->begin();
				m_state_manager->reset();
				reset_state();
			}
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

		return cmd;
	}

	VulkanContext& VulkanContext::execute(RHICommandHandle* handle)
	{
		m_cmd->executeCommands(*static_cast<VulkanCommandHandle*>(handle));

		handle->add_reference();
		m_cmd->add_stagging(handle);
		return *this;
	}

	VulkanContext& VulkanContext::viewport(const RHIViewport& viewport)
	{
		trinex_profile_cpu_n("VulkanContext::viewport");
		m_state_manager->bind(viewport);
		return *this;
	}

	VulkanContext& VulkanContext::scissor(const RHIScissor& scissor)
	{
		trinex_profile_cpu_n("VulkanContext::scissor");
		m_state_manager->bind(scissor);
		return *this;
	}

	VulkanContext& VulkanContext::draw(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		trinex_profile_cpu_n("VulkanContext::draw");
		m_state_manager->flush_graphics(this)->draw(vertex_count, instances, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset,
	                                           size_t instances)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indexed");
		m_state_manager->flush_graphics(this)->drawIndexed(indices_count, instances, indices_offset, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_mesh(uint32_t x, uint32_t y, uint32_t z)
	{
		trinex_profile_cpu_n("VulkanContext::draw_mesh");
		m_state_manager->flush_graphics(this)->drawMeshTasksEXT(x, y, z, API->pfn);
		return *this;
	}

	VulkanContext& VulkanContext::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		trinex_profile_cpu_n("VulkanContext::dispatch");
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
		trinex_profile_cpu_n("VulkanContext::begin_render_pass");
		return m_state_manager->begin_render_pass(this);
	}

	VulkanCommandHandle* VulkanContext::end_render_pass()
	{
		trinex_profile_cpu_n("VulkanContext::end_render_pass");
		return m_state_manager->end_render_pass(this);
	}

	VulkanContext& VulkanContext::shading_rate(RHIShadingRate rate, RHIShadingRateCombiner* combiners)
	{
		trinex_profile_cpu_n("VulkanContext::shading_rate");

		if (!API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)))
			return *this;

		m_state_manager->remove_dirty(VulkanStateManager::ShadingRate);

		vk::Extent2D extent;
		extent.width  = rate.width();
		extent.height = rate.height();

		vk::FragmentShadingRateCombinerOpKHR ops[2] = {
		        VulkanEnums::shading_rate_combiner_of(combiners[0]),
		        VulkanEnums::shading_rate_combiner_of(combiners[1]),
		};

		m_cmd->setFragmentShadingRateKHR(extent, ops, API->pfn);
		return *this;
	}


	void VulkanContext::destroy()
	{
		trx_delete this;
	}

	RHIContext* VulkanAPI::create_context(RHIContextFlags flags)
	{
		return trx_new VulkanContext(flags);
	}
}// namespace Engine
