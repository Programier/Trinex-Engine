#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>

namespace Engine
{
	void VulkanResourceView::release(vk::ImageView view)
	{
		API->m_device.destroyImageView(view);
	}

	VulkanTextureSRV& VulkanTextureSRV::update_descriptor(vk::WriteDescriptorSet& descriptor, VulkanSampler* sampler)
	{
		m_texture->change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::DescriptorImageInfo image_info({}, m_view, vk::ImageLayout::eShaderReadOnlyOptimal);

		if (sampler)
		{
			image_info.setSampler(sampler->m_sampler);
			descriptor.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		}
		else
		{
			descriptor.setDescriptorType(vk::DescriptorType::eSampledImage);
		}

		descriptor.setImageInfo(image_info);
		API->m_device.updateDescriptorSets(descriptor, {});
		return *this;
	}

	VulkanTextureUAV& VulkanTextureUAV::update_descriptor(vk::WriteDescriptorSet& descriptor)
	{
		m_texture->change_layout(vk::ImageLayout::eGeneral);
		vk::DescriptorImageInfo image_info({}, m_view, vk::ImageLayout::eGeneral);
		descriptor.setDescriptorType(vk::DescriptorType::eStorageImage).setImageInfo(image_info);
		API->m_device.updateDescriptorSets(descriptor, {});
		return *this;
	}

	VulkanTextureRTV::~VulkanTextureRTV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			delete rt;
		}
	}

	void VulkanTextureRTV::clear(const LinearColor& color)
	{
		auto cmd = API->end_render_pass();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearColorValue value;
		value.setFloat32({color.r, color.g, color.b, color.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(0)
		        .setBaseMipLevel(0)
		        .setLayerCount(1)
		        .setLevelCount(1);

		API->current_command_buffer_handle().clearColorImage(image(), layout(), value, range);
		cmd->add_object(owner());
	}

	template<typename Target>
	static void blit_target(Target* src, Target* dst, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter,
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

		cmd->m_cmd.blitImage(src->image(), src->layout(), dst->image(), vk::ImageLayout::eTransferDstOptimal, blit,
		                     VulkanEnums::filter_of(filter));

		cmd->add_object(dst->owner());
		cmd->add_object(src->owner());
	}

	void VulkanTextureRTV::blit(RHI_RenderTargetView* texture, const Rect2D& src_rect, const Rect2D& dst_rect,
	                            SamplerFilter filter)
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
	}

	void VulkanTextureDSV::clear(float depth, byte stencil)
	{
		auto cmd = API->end_render_pass();

		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearDepthStencilValue value;
		value.setDepth(depth).setStencil(stencil);
		vk::ImageSubresourceRange range;
		range.setAspectMask(m_texture->aspect()).setBaseArrayLayer(0).setBaseMipLevel(0).setLayerCount(1).setLevelCount(1);

		API->current_command_buffer_handle().clearDepthStencilImage(image(), layout(), value, range);
		cmd->add_object(owner());
	}

	void VulkanTextureDSV::blit(RHI_DepthStencilView* texture, const Rect2D& src_rect, const Rect2D& dst_rect,
	                            SamplerFilter filter)
	{
		auto src = static_cast<VulkanTextureDSV*>(texture);
		blit_target(src, this, src_rect, dst_rect, filter, vk::ImageAspectFlagBits::eDepth);
	}

	VulkanAPI& VulkanAPI::bind_srv(RHI_ShaderResourceView* view, byte slot, RHI_Sampler* sampler)
	{
		if (m_state.m_pipeline)
		{
			m_state.m_pipeline->bind_srv(static_cast<VulkanSRV*>(view), slot, static_cast<VulkanSampler*>(sampler));
		}

		return *this;
	}

	VulkanAPI& VulkanAPI::bind_uav(RHI_UnorderedAccessView* view, byte slot)
	{
		if (m_state.m_pipeline)
		{
			m_state.m_pipeline->bind_uav(static_cast<VulkanUAV*>(view), slot);
		}
		return *this;
	}

	VulkanBufferSRV& VulkanBufferSRV::update_descriptor_base(vk::WriteDescriptorSet& descriptor, vk::DescriptorType type)
	{
		vk::DescriptorBufferInfo buffer_info(m_buffer->m_buffer, 0, m_buffer->m_size);
		descriptor.setDescriptorType(type).setBufferInfo(buffer_info);
		API->m_device.updateDescriptorSets(descriptor, {});
		return *this;
	}

	RHI_Object* VulkanBufferSRV::owner()
	{
		return m_buffer;
	}

	VulkanBufferUAV& VulkanBufferUAV::update_descriptor_base(vk::WriteDescriptorSet& descriptor, vk::DescriptorType type)
	{
		vk::DescriptorBufferInfo buffer_info(m_buffer->m_buffer, 0, m_buffer->m_size);
		descriptor.setDescriptorType(type).setBufferInfo(buffer_info);
		API->m_device.updateDescriptorSets(descriptor, {});
		return *this;
	}

	RHI_Object* VulkanBufferUAV::owner()
	{
		return m_buffer;
	}
}// namespace Engine
