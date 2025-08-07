#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/math/math.hpp>
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

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescSRV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescSRV>();

		ViewDesc desc;

		desc.first_array_slice = Math::min<uint16_t>(view->first_array_slice, texture->layer_count() - 1);
		desc.array_size        = Math::min<uint16_t>(view->array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = Math::min<uint8_t>(view->first_mip, texture->mipmap_count() - 1);
		desc.mip_levels        = Math::min<uint8_t>(view->mip_levels, texture->mipmap_count() - desc.first_mip);
		desc.view_type         = view->view_type == RHITextureType::Undefined ? texture->texture_type() : view->view_type;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescUAV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescUAV>();

		ViewDesc desc;

		desc.first_array_slice = Math::min<uint16_t>(view->first_array_slice, texture->layer_count() - 1);
		desc.array_size        = Math::min<uint16_t>(view->array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = Math::min<uint8_t>(view->mip_slice, texture->mipmap_count() - 1);
		desc.mip_levels        = 1;
		desc.view_type         = view->view_type == RHITextureType::Undefined ? texture->texture_type() : view->view_type;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescRTV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescRTV>();

		ViewDesc desc;

		desc.first_array_slice = Math::min<uint16_t>(view->first_array_slice, texture->layer_count() - 1);
		desc.array_size        = Math::min<uint16_t>(view->array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = Math::min<uint8_t>(view->mip_slice, texture->mipmap_count() - 1);
		desc.mip_levels        = 1;
		desc.view_type         = view->view_type == RHITextureType::Undefined ? texture->texture_type() : view->view_type;

		return desc;
	}

	FORCE_INLINE VulkanTexture::ViewDesc VulkanTexture::ViewDesc::from(const RHITextureDescDSV* view,
	                                                                   const VulkanTexture* texture)
	{
		if (view == nullptr)
			view = &default_value_of<RHITextureDescDSV>();

		ViewDesc desc;

		desc.first_array_slice = Math::min<uint16_t>(view->first_array_slice, texture->layer_count() - 1);
		desc.array_size        = Math::min<uint16_t>(view->array_size, texture->layer_count() - desc.first_array_slice);
		desc.first_mip         = Math::min<uint8_t>(view->mip_slice, texture->mipmap_count() - 1);
		desc.mip_levels        = 1;
		desc.view_type         = view->view_type == RHITextureType::Undefined ? texture->texture_type() : view->view_type;

		return desc;
	}

	VulkanTexture& VulkanTexture::create(RHIColorFormat color_format, Vector3u size, uint32_t mips, RHITextureCreateFlags flags)
	{
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

	RHIShaderResourceView* VulkanTexture::as_srv(RHITextureDescSRV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::ShaderResource))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor | vk::ImageAspectFlagBits::eDepth;
		return static_as_view(this, m_srv, ViewDesc::from(desc, this), mask);
	}

	RHIUnorderedAccessView* VulkanTexture::as_uav(RHITextureDescUAV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::UnorderedAccess))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor;
		return static_as_view(this, m_uav, ViewDesc::from(desc, this), mask);
	}

	RHIRenderTargetView* VulkanTexture::as_rtv(RHITextureDescRTV* desc)
	{
		if (!(m_flags & RHITextureCreateFlags::RenderTarget))
			return nullptr;

		static constexpr auto mask = vk::ImageAspectFlagBits::eColor;
		return static_as_view(this, m_rtv, ViewDesc::from(desc, this), mask);
	}

	RHIDepthStencilView* VulkanTexture::as_dsv(RHITextureDescDSV* desc)
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

		if (m_allocation)
			vmaDestroyImage(API->m_allocator, m_image, m_allocation);
	}

	RHITexture* VulkanAPI::create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
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
				size    = {size.x, 1, Math::max(size.z, 1u)};
				texture = new VulkanTypedTexture<vk::ImageViewType::e1DArray, RHITextureType::Texture1DArray>();
				break;

			case RHITextureType::Texture2D:
				size    = {size.x, size.y, 1};
				texture = new VulkanTypedTexture<vk::ImageViewType::e2D, RHITextureType::Texture2D>();
				break;

			case RHITextureType::Texture2DArray:
				size    = {size.x, size.y, Math::max(size.z, 1u)};
				texture = new VulkanTypedTexture<vk::ImageViewType::e2DArray, RHITextureType::Texture2DArray>();
				break;

			case RHITextureType::TextureCube:
				size    = {size.x, size.y, 6};
				texture = new VulkanTypedTexture<vk::ImageViewType::eCube, RHITextureType::TextureCube>();
				break;

			case RHITextureType::TextureCubeArray:
				size    = {size.x, size.y, Math::max((size.z + 5) / 6, 1u) * 6};
				texture = new VulkanTypedTexture<vk::ImageViewType::eCubeArray, RHITextureType::TextureCubeArray>();
				break;

			case RHITextureType::Texture3D:
				texture = new VulkanTypedTexture<vk::ImageViewType::e3D, RHITextureType::Texture3D>();
				break;
			default: break;
		}

		if (texture)
		{
			texture->create(format, size, Math::max(mips, 1u), flags);
		}
		return texture;
	}

	VulkanAPI& VulkanAPI::update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
	                                     size_t buffer_width, size_t buffer_height)
	{
		auto buffer = API->m_stagging_manager->allocate(size, RHIBufferCreateFlags::TransferSrc);
		buffer->copy(0, static_cast<const byte*>(data), size);

		auto command_buffer           = API->current_command_buffer();
		VulkanTexture* vulkan_texture = static_cast<VulkanTexture*>(texture);

		vulkan_texture->change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy copy_region(
		        0, buffer_width, buffer_height,
		        vk::ImageSubresourceLayers(vulkan_texture->aspect(), region.mip_level, region.array_slice, 1),
		        vk::Offset3D(region.offset.x, region.offset.y, region.offset.z),
		        vk::Extent3D(region.extent.x, region.extent.y, region.extent.z));

		command_buffer->copyBufferToImage(buffer->buffer(), vulkan_texture->image(), vk::ImageLayout::eTransferDstOptimal,
		                                  copy_region);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice,
	                                             const Vector3u& offset, const Vector3u& extent, RHIBuffer* buffer,
	                                             size_t buffer_offset)
	{
		VulkanTexture* src = static_cast<VulkanTexture*>(texture);
		VulkanBuffer* dst  = static_cast<VulkanBuffer*>(buffer);

		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);
		dst->transition(RHIAccess::TransferDst);

		vk::BufferImageCopy region(buffer_offset, 0, 0, vk::ImageSubresourceLayers(src->aspect(), mip_level, array_slice, 1),
		                           vk::Offset3D(offset.x, offset.y, offset.z), vk::Extent3D(extent.x, extent.y, extent.z));
		API->current_command_buffer()->copyImageToBuffer(src->image(), src->layout(), dst->buffer(), region);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture, uint8_t mip_level,
	                                             uint16_t array_slice, const Vector3u& offset, const Vector3u& extent)
	{
		VulkanTexture* dst = static_cast<VulkanTexture*>(texture);
		VulkanBuffer* src  = static_cast<VulkanBuffer*>(buffer);

		src->transition(RHIAccess::TransferDst);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy region(buffer_offset, 0, 0, vk::ImageSubresourceLayers(dst->aspect(), mip_level, array_slice, 1),
		                           vk::Offset3D(offset.x, offset.y, offset.z), vk::Extent3D(extent.x, extent.y, extent.z));
		API->current_command_buffer()->copyBufferToImage(src->buffer(), dst->image(), dst->layout(), region);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_texture_to_texture(RHITexture* src_texture, const RHITextureRegion& src_region,
	                                              RHITexture* dst_texture, const RHITextureRegion& dst_region)
	{
		end_render_pass();
		VulkanTexture* src = static_cast<VulkanTexture*>(src_texture);
		VulkanTexture* dst = static_cast<VulkanTexture*>(dst_texture);

		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		vk::ImageSubresourceLayers src_subresource(src->aspect(), src_region.mip_level, src_region.array_slice, 1);
		vk::ImageSubresourceLayers dst_subresource(dst->aspect(), dst_region.mip_level, dst_region.array_slice, 1);

		if (src_region.extent == dst_region.extent)
		{
			vk::ImageCopy region(src_subresource, vk::Offset3D(src_region.offset.x, src_region.offset.y, src_region.offset.z),
			                     dst_subresource, vk::Offset3D(dst_region.offset.x, dst_region.offset.y, dst_region.offset.z),
			                     vk::Extent3D(src_region.extent.x, src_region.extent.y, src_region.extent.z));

			current_command_buffer()->copyImage(src->image(), src->layout(), dst->image(), dst->layout(), region);
		}
		else
		{
			Vector3u src_region_end = src_region.offset + src_region.extent;
			Vector3u dst_region_end = dst_region.offset + dst_region.extent;

			std::array<vk::Offset3D, 2> src_offsets = {
			        vk::Offset3D(src_region.offset.x, src_region.offset.y, src_region.offset.z),
			        vk::Offset3D(src_region_end.x, src_region_end.y, src_region_end.z),
			};

			std::array<vk::Offset3D, 2> dst_offsets = {
			        vk::Offset3D(dst_region.offset.x, dst_region.offset.y, dst_region.offset.z),
			        vk::Offset3D(dst_region_end.x, dst_region_end.y, dst_region_end.z),
			};

			vk::ImageBlit blit(src_subresource, src_offsets, dst_subresource, dst_offsets);

			current_command_buffer()->blitImage(src->image(), src->layout(), dst->image(), vk::ImageLayout::eTransferDstOptimal,
			                                    blit, vk::Filter::eLinear);
		}

		return *this;
	}


	VulkanAPI& VulkanAPI::barrier(RHITexture* texture, RHIAccess dst_access)
	{
		static_cast<VulkanTexture*>(texture)->change_layout(VulkanEnums::image_layout_of(dst_access));
		return *this;
	}
}// namespace Engine
