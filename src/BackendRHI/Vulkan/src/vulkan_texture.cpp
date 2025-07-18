#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture_2D.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	template<typename Value>
	static FORCE_INLINE Value* static_as_view(VulkanTexture* texture, Vector<VulkanTexture::View<Value>>& views,
	                                          const VulkanTexture::ViewDesc& desc, vk::ImageAspectFlags aspect)
	{
		for (auto& view : views)
		{
			if (view.desc == desc)
				return view.value;
		}

		aspect &= texture->aspect();

		if (aspect == vk::ImageAspectFlagBits::eNone)
			return nullptr;

		vk::ImageSubresourceRange range(aspect, desc.first_mip, desc.mip_levels, desc.first_array_slice, desc.array_size);
		vk::ImageViewCreateInfo view_info({}, texture->image(), VulkanEnums::image_view_type_of(desc.view_type),
		                                  texture->format(), {}, range);
		vk::ImageView view = API->m_device.createImageView(view_info);

		auto& node = views.emplace_back();
		node.desc  = desc;
		node.value = Engine::allocate<Value>(texture, view);
		return node.value;
	}

	template<typename Value>
	static FORCE_INLINE void static_destroy_view(Vector<VulkanTexture::View<Value>>& views)
	{
		for (auto& view : views)
		{
			Engine::release(view.value);
		}
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescSRV& view,
	                                                                   const VulkanTexture* texture)
	{
		ViewDesc desc;

		desc.first_array_slice = glm::min<uint16_t>(view.first_array_slice, texture->layer_count() - 1);
		desc.array_size        = glm::min<uint16_t>(view.array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = glm::min<uint8_t>(view.first_mip, texture->mipmap_count() - 1);
		desc.mip_levels        = glm::min<uint8_t>(view.mip_levels, texture->mipmap_count() - desc.first_mip);
		desc.view_type         = view.view_type == RHITextureType::Undefined ? texture->texture_type() : view.view_type;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescUAV& view,
	                                                                   const VulkanTexture* texture)
	{
		ViewDesc desc;

		desc.first_array_slice = glm::min<uint16_t>(view.first_array_slice, texture->layer_count() - 1);
		desc.array_size        = glm::min<uint16_t>(view.array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = glm::min<uint8_t>(view.mip_slice, texture->mipmap_count() - 1);
		desc.mip_levels        = 1;
		desc.view_type         = view.view_type == RHITextureType::Undefined ? texture->texture_type() : view.view_type;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescRTV& view,
	                                                                   const VulkanTexture* texture)
	{
		ViewDesc desc;

		desc.first_array_slice = glm::min<uint16_t>(view.first_array_slice, texture->layer_count() - 1);
		desc.array_size        = glm::min<uint16_t>(view.array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = glm::min<uint8_t>(view.mip_slice, texture->mipmap_count() - 1);
		desc.mip_levels        = 1;
		desc.view_type         = view.view_type == RHITextureType::Undefined ? texture->texture_type() : view.view_type;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescDSV& view,
	                                                                   const VulkanTexture* texture)
	{
		ViewDesc desc;

		desc.first_array_slice = glm::min<uint16_t>(view.first_array_slice, texture->layer_count() - 1);
		desc.array_size        = glm::min<uint16_t>(view.array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = glm::min<uint8_t>(view.mip_slice, texture->mipmap_count() - 1);
		desc.mip_levels        = 1;
		desc.view_type         = view.view_type == RHITextureType::Undefined ? texture->texture_type() : view.view_type;

		return desc;
	}

	VulkanTexture& VulkanTexture::create(RHIColorFormat color_format, Vector3u size, uint32_t mips, RHITextureCreateFlags flags)
	{
		m_aspect     = VulkanEnums::aspect_of(color_format);
		m_format     = VulkanEnums::format_of(color_format);
		m_extent     = vk::Extent3D{size.x, size.y, size.z};
		m_mips_count = mips;

		switch (image_view_type())
		{
			case vk::ImageViewType::e1DArray:
			case vk::ImageViewType::e2DArray:
			case vk::ImageViewType::eCube:
			case vk::ImageViewType::eCubeArray: m_layers_count = size.z; break;
			default: m_layers_count = 1; break;
		}

		m_layout = vk::ImageLayout::eUndefined;
		m_flags  = flags;

		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;

		if ((flags & RHITextureCreateFlags::ShaderResource) == RHITextureCreateFlags::ShaderResource)
			usage |= vk::ImageUsageFlagBits::eSampled;

		if ((flags & RHITextureCreateFlags::UnorderedAccess) == RHITextureCreateFlags::UnorderedAccess)
			usage |= vk::ImageUsageFlagBits::eStorage;

		if ((flags & RHITextureCreateFlags::RenderTarget) == RHITextureCreateFlags::RenderTarget)
			usage |= vk::ImageUsageFlagBits::eColorAttachment;

		if ((flags & RHITextureCreateFlags::DepthStencilTarget) == RHITextureCreateFlags::DepthStencilTarget)
			usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

		vk::ImageCreateInfo info({}, image_type(), format(), extent(), mipmap_count(), layer_count(), vk::SampleCountFlagBits::e1,
		                         vk::ImageTiling::eOptimal, usage);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

		VkImage out_image = VK_NULL_HANDLE;
		auto res          = vmaCreateImage(API->m_allocator, &static_cast<VkImageCreateInfo&>(info), &alloc_info, &out_image,
		                                   &m_allocation, nullptr);
		trinex_check(res == VK_SUCCESS, "Failed to create texture!");
		m_image = out_image;

		return *this;
	}

	VulkanTexture& VulkanTexture::update(const RHITextureUpdateDesc& desc)
	{
		if (desc.data == nullptr || desc.size == 0)
			return *this;

		auto buffer = API->m_stagging_manager->allocate(desc.size, RHIBufferCreateFlags::TransferSrc);
		buffer->copy(0, static_cast<const byte*>(desc.data), desc.size);

		auto command_buffer = API->current_command_buffer();
		change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy region(0, desc.buffer_width, desc.buffer_height,
		                           vk::ImageSubresourceLayers(aspect(), desc.mip_level, desc.array_slice, 1),
		                           vk::Offset3D(desc.offset.x, desc.offset.y, desc.offset.z),
		                           vk::Extent3D(desc.extent.x, desc.extent.y, desc.extent.z));

		command_buffer->copyBufferToImage(buffer->buffer(), image(), vk::ImageLayout::eTransferDstOptimal, region);
		return *this;
	}

	void VulkanTexture::change_layout(vk::ImageLayout new_layout)
	{
		if (layout() != new_layout || new_layout == vk::ImageLayout::eTransferDstOptimal)
		{
			vk::ImageMemoryBarrier barrier;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout           = m_layout;
			barrier.newLayout           = new_layout;
			barrier.image               = m_image;

			barrier.subresourceRange = vk::ImageSubresourceRange(aspect(), 0, mipmap_count(), 0, layer_count());
			m_layout                 = barrier.newLayout;

			Barrier::transition_image_layout(barrier);

			m_layout = new_layout;
		}
	}

	RHI_ShaderResourceView* VulkanTexture::as_srv(RHITextureDescSRV desc)
	{
		if (!(m_flags & RHITextureCreateFlags::ShaderResource))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor | vk::ImageAspectFlagBits::eDepth;
		return static_as_view(this, m_srv, ViewDesc::from(desc, this), mask);
	}

	RHI_UnorderedAccessView* VulkanTexture::as_uav(RHITextureDescUAV desc)
	{
		if (!(m_flags & RHITextureCreateFlags::UnorderedAccess))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor;
		return static_as_view(this, m_uav, ViewDesc::from(desc, this), mask);
	}

	RHI_RenderTargetView* VulkanTexture::as_rtv(RHITextureDescRTV desc)
	{
		if (!(m_flags & RHITextureCreateFlags::RenderTarget))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor;
		return static_as_view(this, m_rtv, ViewDesc::from(desc, this), mask);
	}

	RHI_DepthStencilView* VulkanTexture::as_dsv(RHITextureDescDSV desc)
	{
		if (!(m_flags & RHITextureCreateFlags::DepthStencilTarget))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		return static_as_view(this, m_dsv, ViewDesc::from(desc, this), mask);
	}

	VulkanTexture::~VulkanTexture()
	{
		static_destroy_view(m_srv);
		static_destroy_view(m_uav);
		static_destroy_view(m_rtv);
		static_destroy_view(m_dsv);

		vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	RHI_Texture* VulkanAPI::create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
	                                       RHITextureCreateFlags flags)
	{
		VulkanTexture* texture = nullptr;

		switch (type)
		{
			case RHITextureType::Texture1D:
				size    = {size.x, 1, 1};
				texture = new VulkanTypedTexture<vk::ImageViewType::e1D, RHITextureType::Texture1D>();
				break;

			case RHITextureType::Texture1DArray:
				size    = {size.x, 1, glm::max(size.z, 1u)};
				texture = new VulkanTypedTexture<vk::ImageViewType::e1DArray, RHITextureType::Texture1DArray>();
				break;

			case RHITextureType::Texture2D:
				size    = {size.x, size.y, 1};
				texture = new VulkanTypedTexture<vk::ImageViewType::e2D, RHITextureType::Texture2D>();
				break;

			case RHITextureType::Texture2DArray:
				size    = {size.x, size.y, glm::max(size.z, 1u)};
				texture = new VulkanTypedTexture<vk::ImageViewType::e2DArray, RHITextureType::Texture2DArray>();
				break;

			case RHITextureType::TextureCube:
				size    = {size.x, size.y, 6};
				texture = new VulkanTypedTexture<vk::ImageViewType::eCube, RHITextureType::TextureCube>();
				break;

			case RHITextureType::TextureCubeArray:
				size    = {size.x, size.y, glm::max((size.z + 5) / 6, 1u) * 6};
				texture = new VulkanTypedTexture<vk::ImageViewType::eCubeArray, RHITextureType::TextureCubeArray>();
				break;

			case RHITextureType::Texture3D:
				texture = new VulkanTypedTexture<vk::ImageViewType::e3D, RHITextureType::Texture3D>();
				break;
			default: break;
		}

		if (texture)
		{
			texture->create(format, size, glm::max(mips, 1u), flags);
		}
		return texture;
	}


	VulkanAPI& VulkanAPI::update_texture(RHI_Texture* texture, const RHITextureUpdateDesc& desc)
	{
		static_cast<VulkanTexture*>(texture)->update(desc);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_texture_to_buffer(RHI_Texture* texture, uint8_t mip_level, uint16_t array_slice,
	                                             const Vector3u& offset, const Vector3u& extent, RHI_Buffer* buffer,
	                                             size_t buffer_offset)
	{
		VulkanTexture* src = static_cast<VulkanTexture*>(texture);
		VulkanBuffer* dst  = static_cast<VulkanBuffer*>(buffer);

		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);
		dst->transition(RHIAccess::CopyDst);

		vk::BufferImageCopy region(buffer_offset, 0, 0, vk::ImageSubresourceLayers(src->aspect(), mip_level, array_slice, 1),
		                           vk::Offset3D(offset.x, offset.y, offset.z), vk::Extent3D(extent.x, extent.y, extent.z));
		API->current_command_buffer()->copyImageToBuffer(src->image(), src->layout(), dst->buffer(), region);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_buffer_to_texture(RHI_Buffer* buffer, size_t buffer_offset, RHI_Texture* texture,
	                                             uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
	                                             const Vector3u& extent)
	{
		VulkanTexture* dst = static_cast<VulkanTexture*>(texture);
		VulkanBuffer* src  = static_cast<VulkanBuffer*>(buffer);

		src->transition(RHIAccess::CopySrc);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy region(buffer_offset, 0, 0, vk::ImageSubresourceLayers(dst->aspect(), mip_level, array_slice, 1),
		                           vk::Offset3D(offset.x, offset.y, offset.z), vk::Extent3D(extent.x, extent.y, extent.z));
		API->current_command_buffer()->copyBufferToImage(src->buffer(), dst->image(), dst->layout(), region);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_texture_to_texture(RHI_Texture* src_texture, const RHITextureRegion& src_region,
	                                              RHI_Texture* dst_texture, const RHITextureRegion& dst_region)
	{
		end_render_pass();
		VulkanTexture* src = static_cast<VulkanTexture*>(src_texture);
		VulkanTexture* dst = static_cast<VulkanTexture*>(dst_texture);

		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		if (src_region.extent == dst_region.extent)
		{
			vk::ImageSubresourceLayers src_subresource(src->aspect(), src_region.mip_level, src_region.array_slice, 1);
			vk::ImageSubresourceLayers dst_subresource(dst->aspect(), dst_region.mip_level, dst_region.array_slice, 1);

			vk::ImageCopy region(src_subresource, vk::Offset3D(src_region.offset.x, src_region.offset.y, src_region.offset.z),
			                     dst_subresource, vk::Offset3D(dst_region.offset.x, dst_region.offset.y, dst_region.offset.z),
			                     vk::Extent3D(src_region.extent.x, src_region.extent.y, src_region.extent.z));

			current_command_buffer()->copyImage(src->image(), src->layout(), dst->image(), dst->layout(), region);
		}
		else
		{
			throw -1;
		}

		return *this;
	}


	VulkanAPI& VulkanAPI::barrier(RHI_Texture* texture, RHIAccess dst_access)
	{
		static_cast<VulkanTexture*>(texture)->change_layout(VulkanEnums::image_layout_of(dst_access));
		return *this;
	}
}// namespace Engine
