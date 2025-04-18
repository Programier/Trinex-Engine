#include <Core/exception.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_texture.hpp>

namespace Engine::Barrier
{
	struct LayoutFlags {
		vk::AccessFlags access;
		vk::PipelineStageFlags stage;

		void setup(vk::ImageLayout layout)
		{
			switch (layout)
			{
				case vk::ImageLayout::eUndefined:
					access = vk::AccessFlagBits::eNone;
					stage  = vk::PipelineStageFlagBits::eTopOfPipe;
					break;

				case vk::ImageLayout::eGeneral:
					access = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite |
					         vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
					stage = all_shaders_stage | vk::PipelineStageFlagBits::eTransfer;
					break;

				case vk::ImageLayout::eColorAttachmentOptimal:
					access = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
					stage  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
					break;

				case vk::ImageLayout::eDepthAttachmentOptimal:
				case vk::ImageLayout::eDepthStencilAttachmentOptimal:
					access = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
					stage  = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
					break;

				case vk::ImageLayout::ePresentSrcKHR:
					access = vk::AccessFlagBits::eNone;
					stage  = vk::PipelineStageFlagBits::eBottomOfPipe;
					break;

				case vk::ImageLayout::eShaderReadOnlyOptimal:
					access = vk::AccessFlagBits::eShaderRead;
					stage  = all_shaders_stage;
					break;

				case vk::ImageLayout::eTransferSrcOptimal:
					access = vk::AccessFlagBits::eTransferRead;
					stage  = vk::PipelineStageFlagBits::eTransfer;
					break;

				case vk::ImageLayout::eTransferDstOptimal:
					access = vk::AccessFlagBits::eTransferWrite;
					stage  = vk::PipelineStageFlagBits::eTransfer;
					break;

				case vk::ImageLayout::eAttachmentOptimal:
					access = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
					stage  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
					break;

				case vk::ImageLayout::eReadOnlyOptimal:
					access = vk::AccessFlagBits::eShaderRead;
					stage  = all_shaders_stage;
					break;

				default:
					throw EngineException("Undefined layout");
			}
		}
	};

	void transition_image_layout(vk::ImageMemoryBarrier& barrier)
	{
		auto cmd = API->end_render_pass();

		LayoutFlags src;
		LayoutFlags dst;

		src.setup(barrier.oldLayout);
		dst.setup(barrier.newLayout);

		barrier.srcAccessMask = src.access;
		barrier.dstAccessMask = dst.access;
		cmd->m_cmd.pipelineBarrier(src.stage, dst.stage, {}, {}, {}, barrier);
	}
}// namespace Engine::Barrier
