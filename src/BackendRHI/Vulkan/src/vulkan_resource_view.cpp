#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_commands.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	RHIDescriptor VulkanSRV::descriptor() const
	{
		return m_descriptor;
	}

	RHIDescriptor VulkanUAV::descriptor() const
	{
		return m_descriptor;
	}

	VulkanTextureSRV::VulkanTextureSRV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		m_descriptor = API->descriptor_heap()->allocate(view, VulkanDescriptorHeap::SampledImage);
	}

	VulkanTextureSRV::~VulkanTextureSRV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::SampledImage);
		API->m_device.destroyImageView(m_view);
	}

	VulkanSRV& VulkanTextureSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->srv_images.bind(this, index);
		return *this;
	}

	VulkanTextureUAV::VulkanTextureUAV(VulkanTexture* texture, vk::ImageView view) : m_texture(texture), m_view(view)
	{
		m_descriptor = API->descriptor_heap()->allocate(m_view, VulkanDescriptorHeap::StorageImage);
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
			trx_delete rt;
		}

		API->m_device.destroyImageView(m_view);
	}

	void VulkanTextureRTV::clear(const LinearColor& color)
	{
		API->end_render_pass();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearColorValue clear_value;
		clear_value.setFloat32({color.r, color.g, color.b, color.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(m_base_layer)
		        .setLayerCount(m_layer_count)
		        .setBaseMipLevel(m_mip)
		        .setLevelCount(1);

		API->current_command_buffer()->clearColorImage(image(), layout(), clear_value, range);
	}

	void VulkanTextureRTV::clear_uint(const Vector4u& value)
	{
		API->end_render_pass();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearColorValue clear_value;
		clear_value.setUint32({value.r, value.g, value.b, value.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(m_base_layer)
		        .setLayerCount(m_layer_count)
		        .setBaseMipLevel(m_mip)
		        .setLevelCount(1);

		API->current_command_buffer()->clearColorImage(image(), layout(), clear_value, range);
	}

	void VulkanTextureRTV::clear_sint(const Vector4i& value)
	{
		API->end_render_pass();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ClearColorValue clear_value;
		clear_value.setInt32({value.r, value.g, value.b, value.a});

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(m_base_layer)
		        .setLayerCount(m_layer_count)
		        .setBaseMipLevel(m_mip)
		        .setLevelCount(1);

		API->current_command_buffer()->clearColorImage(image(), layout(), clear_value, range);
	}

	VulkanTextureDSV::~VulkanTextureDSV()
	{
		while (!m_render_targets.empty())
		{
			auto rt = *m_render_targets.begin();
			trx_delete rt;
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
		range.setAspectMask(m_texture->aspect())
		        .setBaseArrayLayer(m_base_layer)
		        .setLayerCount(m_layer_count)
		        .setBaseMipLevel(m_mip)
		        .setLevelCount(1);

		API->current_command_buffer()->clearDepthStencilImage(image(), layout(), value, range);
	}

	VulkanStorageBufferSRV::VulkanStorageBufferSRV(VulkanBuffer* buffer, uint32_t offset, uint32_t size)
	    : VulkanBufferSRV(buffer, offset, size)
	{
		m_descriptor = API->descriptor_heap()->allocate(buffer, offset, size, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanStorageBufferSRV::~VulkanStorageBufferSRV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanSRV& VulkanStorageBufferSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->storage_buffers.bind(VulkanStateManager::Buffer(buffer()->buffer(), size(), offset()), index);
		return *this;
	}

	VulkanUniformTexelBufferSRV::VulkanUniformTexelBufferSRV(VulkanBuffer* buffer, uint32_t offset, uint32_t size)
	    : VulkanBufferSRV(buffer, offset, size)
	{
		m_descriptor = API->descriptor_heap()->allocate(buffer, offset, size, VulkanDescriptorHeap::UniformTexelBuffer);
	}

	VulkanUniformTexelBufferSRV::~VulkanUniformTexelBufferSRV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::UniformTexelBuffer);
	}

	VulkanSRV& VulkanUniformTexelBufferSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->uniform_texel_buffers.bind(VulkanStateManager::Buffer(buffer()->buffer(), size(), offset()), index);
		return *this;
	}

	VulkanBufferUAV::VulkanBufferUAV(VulkanBuffer* buffer, uint32_t offset, uint32_t size)
	    : m_buffer(buffer), m_offset(offset), m_size(size)
	{
		m_descriptor = API->descriptor_heap()->allocate(buffer, offset, size, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanBufferUAV::~VulkanBufferUAV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanUAV& VulkanBufferUAV::bind(VulkanStateManager* manager, byte index)
	{
		manager->storage_buffers.bind(VulkanStateManager::Buffer(buffer()->buffer(), m_size, m_offset), index);
		return *this;
	}

	VulkanAPI& VulkanAPI::bind_srv(RHIShaderResourceView* view, byte slot)
	{
		static_cast<VulkanSRV*>(view)->bind(API->m_state_manager, slot);
		return *this;
	}

	VulkanAPI& VulkanAPI::bind_uav(RHIUnorderedAccessView* view, byte slot)
	{
		static_cast<VulkanUAV*>(view)->bind(API->m_state_manager, slot);
		return *this;
	}
}// namespace Engine
