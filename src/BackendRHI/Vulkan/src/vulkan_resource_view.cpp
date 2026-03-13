#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>

namespace Trinex
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

	VulkanSRV& VulkanTextureSRV::bind(VulkanContext* context, u8 index)
	{
		context->srv_images.bind(this, index);
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

	VulkanUAV& VulkanTextureUAV::bind(VulkanContext* context, u8 index)
	{
		context->uav_images.bind(this, index);
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

	VulkanSRV& VulkanStorageBufferSRV::bind(VulkanContext* context, u8 index)
	{
		context->storage_buffers.bind(buffer()->buffer(), index);
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

	VulkanSRV& VulkanUniformTexelBufferSRV::bind(VulkanContext* context, u8 index)
	{
		context->uniform_texel_buffers.bind(buffer()->buffer(), index);
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

	VulkanUAV& VulkanBufferUAV::bind(VulkanContext* context, u8 index)
	{
		context->storage_buffers.bind(buffer()->buffer(), index);
		return *this;
	}

	VulkanContext& VulkanContext::bind_srv(RHIShaderResourceView* view, u8 slot)
	{
		static_cast<VulkanSRV*>(view)->bind(this, slot);
		return *this;
	}

	VulkanContext& VulkanContext::bind_uav(RHIUnorderedAccessView* view, u8 slot)
	{
		static_cast<VulkanUAV*>(view)->bind(this, slot);
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

	VulkanContext& VulkanContext::clear_rtv(RHIRenderTargetView* rtv, f32 r, f32 g, f32 b, f32 a)
	{
		return clear_rtv(rtv, std::array<float, 4>({r, g, b, a}));
	}

	VulkanContext& VulkanContext::clear_urtv(RHIRenderTargetView* rtv, u32 r, u32 g, u32 b, u32 a)
	{
		return clear_rtv(rtv, std::array<u32, 4>({r, g, b, a}));
	}

	VulkanContext& VulkanContext::clear_irtv(RHIRenderTargetView* rtv, i32 r, i32 g, i32 b, i32 a)
	{
		return clear_rtv(rtv, std::array<i32, 4>({r, g, b, a}));
	}

	VulkanContext& VulkanContext::clear_dsv(RHIDepthStencilView* dsv, f32 depth, u8 stencil)
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
}// namespace Trinex
