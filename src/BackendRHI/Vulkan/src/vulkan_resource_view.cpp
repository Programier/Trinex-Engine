#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
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
		API->m_device.destroyImageView(m_view);
	}

	VulkanTextureDSV::~VulkanTextureDSV()
	{
		API->m_device.destroyImageView(m_view);
	}

	VulkanStorageBufferSRV::VulkanStorageBufferSRV(VulkanBuffer* buffer) : VulkanBufferSRV(buffer)
	{
		m_descriptor = API->descriptor_heap()->allocate(buffer, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanStorageBufferSRV::~VulkanStorageBufferSRV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanSRV& VulkanStorageBufferSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->storage_buffers.bind(buffer()->buffer(), index);
		return *this;
	}

	VulkanUniformTexelBufferSRV::VulkanUniformTexelBufferSRV(VulkanBuffer* buffer) : VulkanBufferSRV(buffer)
	{
		m_descriptor = API->descriptor_heap()->allocate(buffer, VulkanDescriptorHeap::UniformTexelBuffer);
	}

	VulkanUniformTexelBufferSRV::~VulkanUniformTexelBufferSRV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::UniformTexelBuffer);
	}

	VulkanSRV& VulkanUniformTexelBufferSRV::bind(VulkanStateManager* manager, byte index)
	{
		manager->uniform_texel_buffers.bind(buffer()->buffer(), index);
		return *this;
	}

	VulkanBufferUAV::VulkanBufferUAV(VulkanBuffer* buffer) : m_buffer(buffer)
	{
		m_descriptor = API->descriptor_heap()->allocate(buffer, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanBufferUAV::~VulkanBufferUAV()
	{
		API->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::StorageBuffer);
	}

	VulkanUAV& VulkanBufferUAV::bind(VulkanStateManager* manager, byte index)
	{
		manager->storage_buffers.bind(buffer()->buffer(), index);
		return *this;
	}

	VulkanContext& VulkanContext::bind_srv(RHIShaderResourceView* view, byte slot)
	{
		static_cast<VulkanSRV*>(view)->bind(m_state_manager, slot);
		return *this;
	}

	VulkanContext& VulkanContext::bind_uav(RHIUnorderedAccessView* view, byte slot)
	{
		static_cast<VulkanUAV*>(view)->bind(m_state_manager, slot);
		return *this;
	}

	VulkanContext& VulkanContext::clear_rtv(RHIRenderTargetView* rtv, const vk::ClearColorValue& color)
	{
		VulkanTextureRTV* view = static_cast<VulkanTextureRTV*>(rtv);

		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
		        .setBaseArrayLayer(view->base_layer())
		        .setLayerCount(view->layer_count())
		        .setBaseMipLevel(view->mip())
		        .setLevelCount(1);

		m_cmd->clearColorImage(view->image(), vk::ImageLayout::eTransferDstOptimal, color, range);
		return *this;
	}

	VulkanContext& VulkanContext::clear_rtv(RHIRenderTargetView* rtv, float_t r, float_t g, float_t b, float_t a)
	{
		return clear_rtv(rtv, std::array<float, 4>({r, g, b, a}));
	}

	VulkanContext& VulkanContext::clear_urtv(RHIRenderTargetView* rtv, uint_t r, uint_t g, uint_t b, uint_t a)
	{
		return clear_rtv(rtv, std::array<uint32_t, 4>({r, g, b, a}));
	}

	VulkanContext& VulkanContext::clear_irtv(RHIRenderTargetView* rtv, int_t r, int_t g, int_t b, int_t a)
	{
		return clear_rtv(rtv, std::array<int32_t, 4>({r, g, b, a}));
	}

	VulkanContext& VulkanContext::clear_dsv(RHIDepthStencilView* dsv, float_t depth, byte stencil)
	{
		VulkanTextureDSV* view = static_cast<VulkanTextureDSV*>(dsv);
		VulkanTexture* texture = view->texture();

		vk::ClearDepthStencilValue value;
		value.setDepth(depth).setStencil(stencil);
		vk::ImageSubresourceRange range;
		range.setAspectMask(texture->aspect())
		        .setBaseArrayLayer(view->base_layer())
		        .setLayerCount(view->layer_count())
		        .setBaseMipLevel(view->mip())
		        .setLevelCount(1);

		m_cmd->clearDepthStencilImage(view->image(), vk::ImageLayout::eTransferDstOptimal, value, range);
		return *this;
	}
}// namespace Engine
