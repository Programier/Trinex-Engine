#include <Core/exception.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>

namespace Engine
{
	VulkanCommandBuffer::VulkanCommandBuffer()
	{
		vk::CommandBufferAllocateInfo alloc_info(API->m_command_pool, vk::CommandBufferLevel::ePrimary, 1);
		m_cmd   = API->m_device.allocateCommandBuffers(alloc_info).front();
		m_fence = API->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other)
	    : m_references(std::move(other.m_references)), m_cmd(std::move(other.m_cmd))
	{
		other.m_cmd = vk::CommandBuffer{};
	}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other)
	{
		if (this == &other)
			return *this;

		m_references = std::move(other.m_references);
		m_cmd        = std::move(other.m_cmd);
		other.m_cmd  = vk::CommandBuffer{};
		return *this;
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

	VulkanCommandBuffer& VulkanCommandBuffer::begin()
	{
		trinex_check(m_state == State::Submitted, "Vulkan cmd state must be in Submitted mode");

		while (vk::Result::eTimeout == API->m_device.waitForFences(m_fence, VK_TRUE, UINT64_MAX))
		{
		}

		API->m_device.resetFences(m_fence);
		
		m_cmd.reset();
		release_references();

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

		auto state = rt->state();
		vk::Rect2D area({0, 0}, {static_cast<uint32_t>(state->m_size.x), static_cast<uint32_t>(state->m_size.y)});
		vk::RenderPassBeginInfo info(state->m_render_pass->m_render_pass, rt->m_framebuffer, area);
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

	VulkanCommandBuffer& VulkanCommandBuffer::submit(const vk::SubmitInfo& info)
	{
		trinex_check(has_ended(), "Command Buffer must be in ended state!");
		API->m_graphics_queue.submit(info, m_fence);
		m_state = State::Submitted;
		return *this;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		release_references();
		DESTROY_CALL(destroyFence, m_fence);
		API->m_device.freeCommandBuffers(API->m_command_pool, m_cmd);
	}


}// namespace Engine
