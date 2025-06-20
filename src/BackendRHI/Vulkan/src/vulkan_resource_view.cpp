#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	VulkanTextureSRV::~VulkanTextureSRV()
	{
		API->m_device.destroyImageView(m_view);
	}

	VulkanSRV& VulkanTextureSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->srv_images.bind(this, index);
		return *this;
	}

	VulkanTextureUAV::~VulkanTextureUAV()
	{
		API->m_device.destroyImageView(m_view);
	}

	VulkanUAV& VulkanTextureUAV::bind(VulkanStateManager* manager, byte index)
	{
		manager->uav_images.bind(this, index);
		return *this;
	}

	VulkanTextureRTV::~VulkanTextureRTV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}

		API->m_device.destroyImageView(m_view);
	}

	void VulkanTextureRTV::clear(const LinearColor& color)
	{
		API->end_render_pass();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearColorValue value;
		value.setFloat32({color.r, color.g, color.b, color.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(0)
		        .setBaseMipLevel(0)
		        .setLayerCount(1)
		        .setLevelCount(1);

		API->current_command_buffer()->clearColorImage(image(), layout(), value, range);
	}

	template<typename Target>
	static void blit_target(Target* src, Target* dst, const RHIRect& src_rect, const RHIRect& dst_rect, RHISamplerFilter filter,
	                        vk::ImageAspectFlagBits aspect)
	{
		auto cmd = API->end_render_pass();
		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		auto src_end = src_rect.pos + src_rect.size;
		auto dst_end = dst_rect.pos + dst_rect.size;

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.pos.x, src_rect.pos.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.pos.x, dst_rect.pos.y, 0), vk::Offset3D(dst_end.x, dst_end.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(aspect, 0, 0, 1));
		blit.setDstSubresource(vk::ImageSubresourceLayers(aspect, 0, 0, 1));

		cmd->blitImage(src->image(), src->layout(), dst->image(), vk::ImageLayout::eTransferDstOptimal, blit,
		               VulkanEnums::filter_of(filter));
	}

	void VulkanTextureRTV::blit(RHI_RenderTargetView* texture, const RHIRect& src_rect, const RHIRect& dst_rect,
	                            RHISamplerFilter filter)
	{
		auto src = static_cast<VulkanTextureRTV*>(texture);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eColor);
	}

	VulkanTextureDSV::~VulkanTextureDSV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}

		API->m_device.destroyImageView(m_view);
	}

	void VulkanTextureDSV::clear(float depth, byte stencil)
	{
		API->end_render_pass();

		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearDepthStencilValue value;
		value.setDepth(depth).setStencil(stencil);
		vk::ImageSubresourceRange range;
		range.setAspectMask(m_texture->aspect()).setBaseArrayLayer(0).setBaseMipLevel(0).setLayerCount(1).setLevelCount(1);

		API->current_command_buffer()->clearDepthStencilImage(image(), layout(), value, range);
	}

	VulkanSRV& VulkanStorageBufferSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->storage_buffers.bind(VulkanStateManager::Buffer(buffer()->buffer(), buffer()->size(), 0), index);
		return *this;
	}

	VulkanSRV& VulkanUniformTexelBufferSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->uniform_texel_buffers.bind(VulkanStateManager::Buffer(buffer()->buffer(), buffer()->size(), 0), index);
		return *this;
	}

	VulkanUAV& VulkanBufferUAV::bind(VulkanStateManager* manager, byte index)
	{
		manager->storage_buffers.bind(VulkanStateManager::Buffer(buffer()->buffer(), buffer()->size(), 0), index);
		return *this;
	}

	void VulkanTextureDSV::blit(RHI_DepthStencilView* texture, const RHIRect& src_rect, const RHIRect& dst_rect,
	                            RHISamplerFilter filter)
	{
		auto src = static_cast<VulkanTextureDSV*>(texture);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eDepth);
	}

	VulkanAPI& VulkanAPI::bind_srv(RHI_ShaderResourceView* view, byte slot)
	{
		static_cast<VulkanSRV*>(view)->bind(API->m_state_manager, slot);
		return *this;
	}

	VulkanAPI& VulkanAPI::bind_uav(RHI_UnorderedAccessView* view, byte slot)
	{
		static_cast<VulkanUAV*>(view)->bind(API->m_state_manager, slot);
		return *this;
	}
}// namespace Engine
